#include <stdbool.h>
#include <stdlib.h>
#include "../header/util.h"
#include "../header/compute_edges_omp.h"
#include <omp.h>

/* -------------------------------------------------------------------------- --
   FUNCTION:
   get_edges_to_optimize_for_budget

   --------------------------------------------------------------------------
   Purpose:
   get all the edges that, if improved, would have the biggest impact until
   the budget is reached.

   --------------------------------------------------------------------------
   Description:
   This function "improves" an edge, recalculate the paths who have the said
   edge within their visibility and compare the new dijkstra "cost" to the old
   cost of the paths.
   If the costs is smaller, it is added to the total costs saved by improving
   the edge.
   If the best edge improvement of the graph had a non-null impact, the graph is then
   updated to take in this improvement. The function stops when the budget
   is reached or that no other edges improvement has an impact.

   --------------------------------------------------------------------------
   Return value:
   void

-- -------------------------------------------------------------------------- */
int get_edges_to_optimize_for_budget_omp(config_t *config, long double *budget_used, double_unsigned_list_t **selected_edges)
{
    graph_t graph;
    path_t **paths;
    unsigned_list_t **path_list =NULL;
    int ret_code;
    (*selected_edges) = NULL;
    long double max_saved_cost;
    uint32_t nb_paths;
    long double budget_left = config->budget;
    int32_t edge_id_to_optimize;
    double *dijkstra_backward_dist, *dijkstra_forward_dist;
    long double new_dijkstra_cost, dijkstra_cost, cost_difference;
    bool stop = false;

    ret_code = get_graph(config, &graph);
    if (ret_code != OK)
    {
        return ret_code;
    }
    ret_code = get_paths(config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        free_graph(&graph);
        return ret_code;
    }

    bool *impact = calloc(nb_paths, sizeof(bool));
    if (impact == NULL)
    {
        free_graph(&graph);
        free_paths(paths, nb_paths);
        fprintf(stderr, "Memory allocation failed for impact array\n");
        return MEMORY_ALLOC_ERROR;
    }

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        impact[i] = true;
    }

    // This array is used to get the cost saved by improving an edge
    // for a given path
    // we need to know the path to only reset its cost saved if
    // the best edge for a given iteration was in the path's
    // visibility
    double_unsigned_list_t **cost_diff_array;
    ret_code = init_cost_diff_array(&cost_diff_array, nb_paths);
    if (ret_code != OK)
    {
        free_graph(&graph);
        free_paths(paths, nb_paths);
        free(impact);
        fprintf(stderr, "Memory allocation failed for cost_diff_array array\n");
        return MEMORY_ALLOC_ERROR;
    }

    omp_set_num_threads(config->thread_number);
    unsigned_list_t *current_path;
    uint32_t t_it;
    uint32_t path_id;
    while (!stop)
    {
        assign_traces_to_threads(config, paths, nb_paths, &path_list, impact);
        // printf("let's go\n");
// used to know the cost difference, the optimization of the edge would
// bring for the path where the edge is in the visibility
#pragma omp parallel shared(path_list, nb_paths, impact, graph, paths, cost_diff_array) private(path_id, t_it, current_path, dijkstra_forward_dist, dijkstra_backward_dist, dijkstra_cost, new_dijkstra_cost, cost_difference)
        {
            dijkstra_forward_dist = calloc(graph.nb_vertices, sizeof(double));
            dijkstra_backward_dist = calloc(graph.nb_vertices, sizeof(double));

            t_it = omp_get_thread_num();
            current_path = path_list[t_it];
            // printf("thread number %d sur %d\n", t_it,omp_get_num_threads());
            while (current_path != NULL)
            {
                path_id = current_path->u_value;
                impact[path_id] = false;

                //  calculating the dijkstra path backward and forward allows for
                //  a much faster computing of the impact of the improvement of an edge
                dijkstra_cost = dijkstra_backward(&graph, &dijkstra_backward_dist, NULL, paths[path_id]);
                dijkstra_forward(&graph, &dijkstra_forward_dist, NULL, paths[path_id]);

                for (uint32_t edge_id = 0; edge_id < graph.nb_edges; edge_id++)
                {
                    if (graph.edge_array[edge_id]->dist > budget_left)
                    {
                        continue;
                    }
                    //  if the edge's vertexes are in the visibility of the path
                    //  and the edge is not already optimized
                    
                    //  This ensure we keep the already calculated value for the path
                    //  which did not have the chosen edge in their visibility
                    if (edge_is_in_visibilite(paths[path_id], graph.edge_array[edge_id]) && (graph.edge_array[edge_id]->dist != graph.edge_array[edge_id]->danger))
                    {

                        new_dijkstra_cost = updated_dist(graph.edge_array[edge_id], paths[path_id], dijkstra_forward_dist, dijkstra_backward_dist);

                        cost_difference = dijkstra_cost - new_dijkstra_cost;
                        if (cost_difference > 0)
                        {
#pragma omp critical
                            add_double_unsigned_list_t(&cost_diff_array[path_id], edge_id, cost_difference);
                        }
                    }
                }
                current_path = current_path->next;
            }
            free(dijkstra_forward_dist);
            free(dijkstra_backward_dist);
        }
        edge_id_to_optimize = -1;
        get_max_edge_to_optimize(cost_diff_array, nb_paths, graph.nb_edges, graph.edge_array, &edge_id_to_optimize, &max_saved_cost, budget_left);
        if (edge_id_to_optimize == -1)
        {
            stop = true;
        }
        else
        {
            for (uint32_t path_id = 0; path_id < nb_paths; path_id++)
            {
                if (edge_is_in_visibilite(paths[path_id], graph.edge_array[edge_id_to_optimize]))
                {
                    impact[path_id] = true;
                    free_double_unsigned_list_t(cost_diff_array[path_id]);
                    cost_diff_array[path_id] = NULL;
                }
            }
            ret_code = add_double_unsigned_list_t(selected_edges, graph.edge_array[edge_id_to_optimize]->id, max_saved_cost);
            if (ret_code != OK)
            {
                free(impact);
                free_graph(&graph);
                free_paths(paths, nb_paths);
                free_double_unsigned_list_t(*selected_edges);
                return ret_code;
            }
            budget_left = budget_left - graph.edge_array[edge_id_to_optimize]->dist;
            *budget_used = config->budget - budget_left;
            graph.edge_array[edge_id_to_optimize]->danger = graph.edge_array[edge_id_to_optimize]->dist;
        }
    }
    free_path_list(config, path_list);
    free(impact);
    free_graph(&graph);
    free_paths(paths, nb_paths);
    free_cost_diff_array(cost_diff_array, nb_paths);
    return OK;
}