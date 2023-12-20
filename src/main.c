#include "../header/edge.h"
#include "../header/trace.h"
#include "../header/djikstra.h"
#include <stdlib.h>
#include <stdbool.h>

#define BUDGET 10

int main()
{
    vertex_t *graph;
    edge_t** edge_array;
    double * dist_array;
    int nbvertex_ts;
    int nbedge_t;
    int nbTrace;

    get_graph("./data/artificiel/data_graphe.csv", &graph, &edge_array, &nbvertex_ts,&nbedge_t);

    trace_t *traces = get_traces("./data/artificiel/data_path.csv", ";", &nbTrace);
    djikstra_forward(graph, nbvertex_ts, &dist_array, &traces[0]);

    long double budget_left = BUDGET;
    int edge_id_to_optimize;
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
        // used to know the cost difference, the optimization of the edge would bring for each trace
        cost_diff_edge_t cost_diff_array[nbedge_t]; 
        init_cost_diff_array(cost_diff_array,nbedge_t);

        for (int trace_id = 0; trace_id < nbTrace && impact[trace_id]; trace_id++)
        {
            impact[trace_id]=false;
            traces[trace_id].djikstra_dist = djikstra_backward(graph, nbvertex_ts, &traces[trace_id].backward_djikstra, &traces[trace_id]);
            djikstra_forward(graph, nbvertex_ts, &traces[trace_id].foward_djikstra, &traces[trace_id]);

            for (int path_id = 0; path_id < nbedge_t; path_id++)
            {
                //if the edge's vertexes are in the visibility of the trace
                // and the edge is not optimized already
                if(edge_is_in_visibilite(&traces[trace_id], edge_array[path_id]) && (edge_array[path_id]->dist != edge_array[path_id]->danger)){
                    
                    //optimizing an edge is making its danger equal to its distance
                    old_danger = edge_array[path_id]->danger;
                    edge_array[path_id]->danger = edge_array[path_id]->dist;

                    new_djikstra_dist =  updated_dist(edge_array[path_id],&traces[trace_id]);
                    
                    //unoptimize the edge
                    edge_array[path_id]->danger = old_danger;
                    
                    cost_difference = traces[trace_id].djikstra_dist-new_djikstra_dist;
                    if(cost_difference>0){
                        cost_diff_array[path_id].djikstra_cost_diff += cost_difference;
                    }
                }
            }
            // print_cost_diff(cost_diff_array,nbedge_t);
        }
        edge_id_to_optimize=-1;
        get_max_edge_to_optimize(cost_diff_array,nbedge_t,&edge_id_to_optimize,&budget_left);
        if(edge_id_to_optimize==-1){
            stop = true;
        }
        else{
            for (int i = 0; i < nbTrace; i++)
            {
                impact[i] = true;
            }
            budget_left = budget_left-edge_array[edge_id_to_optimize]->dist;
            edge_array[edge_id_to_optimize]->danger = edge_array[edge_id_to_optimize]->dist;
            printf(" %f\n", edge_array[edge_id_to_optimize]->dist);
        }
    }
    return 0;
}