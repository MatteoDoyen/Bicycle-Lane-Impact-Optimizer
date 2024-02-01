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
int get_edges_to_optimize_for_budget_omp(cifre_conf_t *config, long double *budget_used, double_unsigned_list_t **selected_edges)
{
    vertex_t **graph;
    path_t **paths;
    edge_t **edge_array;
    int ret_code;
    (*selected_edges) = NULL;
    long double max_saved_cost;
    uint32_t nb_vertices, nb_edges, nb_paths;
    long double budget_left = config->budget;
    int32_t edge_id_to_optimize;
    double *djikstra_backward_dist, *djikstra_forward_dist;
    long double new_djikstra_cost, djikstra_cost, cost_difference;
    bool stop = false;

    ret_code = get_graph(config, &graph, &edge_array, &nb_vertices, &nb_edges);
    if (ret_code != OK)
    {
        return ret_code;
    }
    ret_code = get_paths(config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        free_edge(edge_array, nb_edges);
        free_graph(graph, nb_vertices);
        return ret_code;
    }

    bool *impact = calloc(nb_paths, sizeof(bool));
    if (impact == NULL)
    {
        free_edge(edge_array, nb_edges);
        free_graph(graph, nb_vertices);
        free_paths(paths, nb_paths);
        fprintf(stderr, "Memory allocation failed for impact array\n");
        return MEMORY_ALLOC_ERROR;
    }

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        impact[i] = true;
    }

    double_unsigned_list_t **cost_diff_array;
    ret_code = init_cost_diff_array(&cost_diff_array, nb_paths);
    if (ret_code != OK)
    {
        free_edge(edge_array, nb_edges);
        free_graph(graph, nb_vertices);
        free_paths(paths, nb_paths);
        free(impact);
        fprintf(stderr, "Memory allocation failed for cost_diff_array array\n");
        return MEMORY_ALLOC_ERROR;
    }
    while (!stop)
    {
// used to know the cost difference, the optimization of the edge would
// bring for the path where the edge is in the visibility
#pragma omp parallel shared(nb_paths,impact, graph, nb_vertices, paths,edge_array,cost_diff_array) private(djikstra_forward_dist, djikstra_backward_dist,djikstra_cost, new_djikstra_cost, cost_difference)
{
        djikstra_forward_dist = calloc(nb_vertices, sizeof(double));
        djikstra_backward_dist = calloc(nb_vertices, sizeof(double));
#pragma omp for
        for (uint32_t path_id = 0; path_id < nb_paths; path_id++)
        {
            if (!impact[path_id])
            {
                continue;
            }
            impact[path_id] = false;

            //  calculating the djikstra path backward and forward allows for
            //  a much faster computing of the impact of the improvement of an edge
            djikstra_cost = djikstra_backward(graph, nb_vertices, &djikstra_backward_dist, NULL, paths[path_id]);
            djikstra_forward(graph, nb_vertices, &djikstra_forward_dist, NULL, paths[path_id]);

            for (uint32_t edge_id = 0; edge_id < nb_edges; edge_id++)
            {
                if (edge_array[edge_id]->dist > budget_left)
                {
                    continue;
                }
                //  if the edge's vertexes are in the visibility of the path
                //  and the edge is not already optimized
                if (edge_is_in_visibilite(paths[path_id], edge_array[edge_id]) && (edge_array[edge_id]->dist != edge_array[edge_id]->danger))
                {

                    new_djikstra_cost = updated_dist(edge_array[edge_id], paths[path_id], djikstra_forward_dist, djikstra_backward_dist);

                    cost_difference = djikstra_cost - new_djikstra_cost;
                    if (cost_difference > 0)
                    {
                        #pragma omp critical
                        add_double_unsigned_list_t(&cost_diff_array[path_id], edge_id, cost_difference);
                    }
                }
            }
        }
        free(djikstra_forward_dist);
        free(djikstra_backward_dist);
#pragma omp barrier
    }
    edge_id_to_optimize = -1;
    get_max_edge_to_optimize(cost_diff_array, nb_paths, nb_edges, edge_array, &edge_id_to_optimize, &max_saved_cost, budget_left);
    if (edge_id_to_optimize == -1)
    {
        stop = true;
    }
    else
    {
        for (uint32_t path_id = 0; path_id < nb_paths; path_id++)
        {
            if (edge_is_in_visibilite(paths[path_id], edge_array[edge_id_to_optimize]))
            {
                impact[path_id] = true;
                free_double_unsigned_list_t(cost_diff_array[path_id]);
                cost_diff_array[path_id] = NULL;
            }
        }
        ret_code = add_double_unsigned_list_t(selected_edges, edge_array[edge_id_to_optimize]->id, max_saved_cost);
        if (ret_code != OK)
        {
            free(impact);
            free_edge(edge_array, nb_edges);
            free_graph(graph, nb_vertices);
            free_paths(paths, nb_paths);
            free_double_unsigned_list_t(*selected_edges);
            return ret_code;
        }
        budget_left = budget_left - edge_array[edge_id_to_optimize]->dist;
        *budget_used = config->budget - budget_left;
        edge_array[edge_id_to_optimize]->danger = edge_array[edge_id_to_optimize]->dist;
    }
}
free(impact);
free_edge(edge_array, nb_edges);
free_graph(graph, nb_vertices);
free_paths(paths, nb_paths);
free_cost_diff_array(cost_diff_array, nb_paths);
return OK;
}