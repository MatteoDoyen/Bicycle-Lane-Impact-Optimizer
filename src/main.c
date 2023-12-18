#include "../header/arc.h"
#include "../header/trace.h"
#include "../header/djikstra.h"
#include <stdlib.h>
#include <stdbool.h>

#define BUDGET 10

int main()
{
    Noeud *graph;
    Arc** arcArray;
    int nbNoeuds;
    int nbArc;

    createGraphFromCSV("./data/artificiel/data_graphe.csv", &graph, &arcArray, &nbNoeuds,&nbArc);
    // printf("salut %d \n",numNoeuds);
   
    // int source = 1538;  // Source vertex
    // int destination = 1106;  // Destination vertex
    double * dist_array;
    int nbTrace;


    trace_t *traces = get_traces("./data/artificiel/data_path.csv", ";", &nbTrace);
    djikstra_forward(graph, nbNoeuds, &dist_array, &traces[0]);

    
    long double budget_left = BUDGET;
    int arc_id_to_optimize;
    double old_danger;
    double new_djikstra_dist;
    double cost_difference;
    bool stop = false;
    bool impact[nbTrace];

    for (int i = 0; i < nbTrace; i++)
    {
        impact[i] = true;
    }
    while(!stop){
        // used to know the cost difference, the optimization of the arc would bring for each trace
        cost_diff_arc_t cost_diff_array[nbArc]; 
        init_cost_diff_array(cost_diff_array,nbArc);

        for (int trace_id = 0; trace_id < nbTrace && impact[trace_id]; trace_id++)
        {
            impact[trace_id]=false;
            traces[trace_id].djikstra_dist = djikstra_backward(graph, nbNoeuds, &traces[trace_id].backward_djikstra, &traces[trace_id]);
            djikstra_forward(graph, nbNoeuds, &traces[trace_id].foward_djikstra, &traces[trace_id]);

            for (int path_id = 0; path_id < nbArc; path_id++)
            {
                //if the edge's vertexes are in the visibility of the trace
                // and the arc is not optimized already
                if(arcIsInVisiblite(&traces[trace_id], arcArray[path_id]) && (arcArray[path_id]->dist != arcArray[path_id]->danger)){
                    // old_dist =  updated_dist(arcArray[path_id],&traces[trace_id]);
                    //if not already optimized
                    
                    //optimizing an edge is making its danger equal to its distance
                    old_danger = arcArray[path_id]->danger;
                    arcArray[path_id]->danger = arcArray[path_id]->dist;

                    new_djikstra_dist =  updated_dist(arcArray[path_id],&traces[trace_id]);
                    
                    //unoptimize the arc
                    arcArray[path_id]->danger = old_danger;
                    
                    cost_difference = traces[trace_id].djikstra_dist-new_djikstra_dist;
                    if(cost_difference>0){
                        // new_cost_diff(trace_id,cost_difference,arcArray[path_id]->dist,&cost_diff_array[path_id]);
                        // print_cost_diff(cost_diff_array,nbArc);
                        // return 0;
                        cost_diff_array[path_id].djikstra_cost_diff += cost_difference;
                    }
                }
            }
            // print_cost_diff(cost_diff_array,nbArc);
        }
        arc_id_to_optimize=-1;
        get_max_arc_to_optimize(cost_diff_array,nbArc,&arc_id_to_optimize,&budget_left);
        if(arc_id_to_optimize==-1){
            stop = true;
        }
        else{
            for (int i = 0; i < nbTrace; i++)
            {
                impact[i] = true;
            }
            budget_left = budget_left-arcArray[arc_id_to_optimize]->dist;
            arcArray[arc_id_to_optimize]->danger = arcArray[arc_id_to_optimize]->dist;
            printf(" %f\n", arcArray[arc_id_to_optimize]->dist);
        }
    }
    return 0;
}