#include "../header/arc.h"
#include "../header/trace.h"
#include "../header/djikstra.h"
#include <stdlib.h>
#include <stdbool.h>

#define BUDGET 250

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
    djikstra(graph, nbNoeuds, &dist_array, &traces[0]);

    
    long double budget_left = BUDGET;
    unsigned int arc_id_to_optimize;
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
            traces[trace_id].djikstra_dist = djikstra(graph, nbNoeuds, &traces[trace_id].backward_djikstra, &traces[trace_id]);
            djikstra(graph, nbNoeuds, &traces[trace_id].foward_djikstra, &traces[trace_id]);

            for (int path_id = 0; path_id < nbArc; path_id++)
            {
                //if the edge's vertexes are in the visibility of the trace
                if(arcIsInVisiblite(&traces[trace_id], arcArray[path_id])){
                    // old_dist =  updated_dist(arcArray[path_id],&traces[trace_id]);
                    //if not already optimized
                    if(arcArray[path_id]->dist != arcArray[path_id]->danger){
                        //optimizing an edge is making its danger equal to its distance
                        arcArray[path_id]->danger = arcArray[path_id]->dist;
                    }
                    new_djikstra_dist =  updated_dist(arcArray[path_id],&traces[trace_id]);
                    cost_difference = traces[trace_id].djikstra_dist-new_djikstra_dist;
                    if(cost_difference>0){
                        new_cost_diff(trace_id,cost_difference,&cost_diff_array[path_id]);
                        // print_cost_diff(cost_diff_array,nbArc);
                        // return 0;
                        break;
                    }
                }
            }
            print_cost_diff(cost_diff_array,nbArc);
            break;
        }
        arc_id_to_optimize=-1;
        get_max_arc_to_optimize(cost_diff_array,nbArc,&arc_id_to_optimize,&budget_left);
        break;
    }
    


    return 0;
}