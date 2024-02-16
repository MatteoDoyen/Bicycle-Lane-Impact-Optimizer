#include <stdbool.h>
#include <stdlib.h>
#include "../header/util.h"
#include "../header/compute_edges_pthread.h"

void *compute_optimize_for_budget_threaded(void *arg)
{
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    graph_t *graph = thread_arg->graph;
    long double new_dijkstra_cost, dijkstra_cost, cost_difference;
    double *dijkstra_backward_dist = (double *)calloc(graph->nb_vertices, sizeof(double));
    double *dijkstra_forward_dist = (double *)calloc(graph->nb_vertices, sizeof(double));

    if (dijkstra_backward_dist == NULL || dijkstra_forward_dist == NULL)
    {
        return (void *)MEMORY_ALLOC_ERROR;
    }

    unsigned_list_t * current_path = *thread_arg->path_list;
    uint32_t path_id;
    // by offsetting the loop index with the number of thread, we are sure that no two thread will ever
    // work on the same path
    while (current_path!=NULL)
    {
        path_id = current_path->u_value;
        thread_arg->impact[path_id] = false;
        dijkstra_cost = dijkstra_forward(graph, &dijkstra_forward_dist,NULL, thread_arg->paths[path_id]);
        dijkstra_backward(graph, &dijkstra_backward_dist, NULL, thread_arg->paths[path_id]);

        for (uint32_t edge_id = 0; edge_id < graph->nb_edges; edge_id++)
        {

            if (graph->edge_array[edge_id]->dist > ((double)*thread_arg->budget_left))
            {
                
                continue;
            }
            // if the edge's vertexes are in the visibility of the path
            //  and the edge is not already optimized
            if (edge_is_in_visibilite(thread_arg->paths[path_id], graph->edge_array[edge_id]) && (graph->edge_array[edge_id]->dist != graph->edge_array[edge_id]->danger))
            {

                new_dijkstra_cost = updated_dist(graph->edge_array[edge_id], thread_arg->paths[path_id], dijkstra_forward_dist, dijkstra_backward_dist);

                cost_difference = dijkstra_cost - new_dijkstra_cost;
                if (cost_difference > 0)
                {
                    // could be improved by creating a mutex for each edge
                    pthread_mutex_lock(thread_arg->mutex);
                    add_double_unsigned_list_t(&thread_arg->cost_diff_array[path_id], edge_id, cost_difference);
                    pthread_mutex_unlock(thread_arg->mutex);
                }
            }
        }
        current_path = current_path->next;
    }
    free(dijkstra_backward_dist);
    free(dijkstra_forward_dist);
    return (void *)OK;
}

int get_edges_to_optimize_for_budget_pthread(config_t * config,long double *budget_used, double_unsigned_list_t **selected_edges)
{
    graph_t graph;
    path_t **paths;
    int ret_code = OK;
    (*selected_edges) = NULL;
    long double budget_left=config->budget;
    long double max_saved_cost;
    uint32_t nb_paths;
    int32_t edge_id_to_optimize;
    bool stop = false;
    pthread_mutex_t mutex_cost_diff_array;
    thread_arg_t *thread_arg;
    pthread_t *threads;
    bool *impact;
    double_unsigned_list_t **cost_diff_array;
    unsigned_list_t ** path_list = NULL;
    int budget_mult = 0;

    pthread_mutex_init(&mutex_cost_diff_array, NULL);

    ret_code = get_graph(config, &graph);
    if (ret_code != OK)
    {
        return ret_code;
    }
    ret_code = get_paths(config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        goto cleanup_edge_graph;
    }
    impact = calloc(nb_paths, sizeof(bool));
    if (impact == NULL)
    {
        fprintf(stderr, "Memory allocation failed for impact array\n");
        ret_code = MEMORY_ALLOC_ERROR;
        goto cleanup_paths;
    }
    ret_code = init_cost_diff_array(&cost_diff_array, nb_paths);
    if (ret_code != OK)
    {
        fprintf(stderr, "Memory allocation failed for cost_diff_array\n");
        goto cleanup_impacts;
    }
    for (uint32_t i = 0; i < nb_paths; i++)
    {
        impact[i] = true;
    }
    thread_arg = calloc(config->thread_number,sizeof(thread_arg_t));
    if (thread_arg == NULL)
    {
        fprintf(stderr, "Memory allocation failed for thread_arg\n");
        goto clean_up_thread_arg;
    }
    threads = calloc(config->thread_number,sizeof(pthread_t));
    if (threads == NULL)
    {
        fprintf(stderr, "Memory allocation failed for threads\n");
        goto clean_up_threads;
    }
    for (uint32_t i = 0; i < config->thread_number; i++)
    {
        thread_arg[i] = (thread_arg_t){
            .cost_diff_array = cost_diff_array,
            .graph = &graph,
            .nb_paths = nb_paths,
            .thread_id = i,
            .offset = config->thread_number,
            .mutex = &mutex_cost_diff_array,
            .paths = paths,
            .impact = impact,
            .budget_left = &budget_left
        };
    }
    while (!stop)
    {
        assign_traces_to_threads(config,paths,nb_paths,&path_list,impact);
        // used to know the cost difference, the optimization of the edge would
        // bring for the path where the edge is in the visibility
        for (uint32_t i= 0; i < config->thread_number; i++)
        {
            thread_arg[i].path_list = &path_list[i];
            pthread_create(&threads[i], NULL, compute_optimize_for_budget_threaded, (void *)&thread_arg[i]);
        }
        for (uint32_t i= 0; i < config->thread_number; i++)
        {
            // wait thread
            pthread_join(threads[i], NULL);
        }
        edge_id_to_optimize = -1;
        get_max_edge_to_optimize(cost_diff_array, nb_paths,graph.nb_edges, graph.edge_array, &edge_id_to_optimize, &max_saved_cost, budget_left);
        if (edge_id_to_optimize == -1)
        {
            stop = true;
        }
        else
        {
            // the next loop will only consider the paths who had the edge in their visibility
            for (uint32_t path_id = 0; path_id < nb_paths; path_id++)
            {
                if (edge_is_in_visibilite(paths[path_id], graph.edge_array[edge_id_to_optimize]))
                {
                    impact[path_id] = true;
                    free_double_unsigned_list_t(cost_diff_array[path_id]);
                    cost_diff_array[path_id] = NULL;
                }
            }
            //add the selected edge to the selected_edge array
            ret_code = add_double_unsigned_list_t(selected_edges,graph.edge_array[edge_id_to_optimize]->id, max_saved_cost);
            if (ret_code != OK)
            {
                free_double_unsigned_list_t(*selected_edges);
                *selected_edges = NULL;
                goto clean_up_cost_diff;
            }
            budget_left = budget_left - graph.edge_array[edge_id_to_optimize]->dist;
            *budget_used = config->budget - budget_left;
            // if((int)(*budget_used/5)>budget_mult){
            //     printf("%Lf %f\n",*budget_used,get_total_saved_cost(*selected_edges));
            //     budget_mult = (int)(*budget_used/5);
            // }
            graph.edge_array[edge_id_to_optimize]->danger = graph.edge_array[edge_id_to_optimize]->dist;            
        }
    }

    free_path_list(config,path_list);
clean_up_cost_diff:
    free_cost_diff_array(cost_diff_array,nb_paths);
clean_up_threads:
    free(threads);
clean_up_thread_arg:
    free(thread_arg);
cleanup_impacts:
    free(impact);
cleanup_paths:
    free_paths(paths, nb_paths);
cleanup_edge_graph:
    pthread_mutex_destroy(&mutex_cost_diff_array);
    free_graph(&graph);
    return OK;
}