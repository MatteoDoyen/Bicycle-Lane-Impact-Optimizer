#include <stdbool.h>
#include <stdlib.h>
#include "../header/util.h"
#include "../header/compute_edges.h"

void save_selected_edges(double_unsigned_list_t *head,char * file_path)
{
    FILE * file = fopen(file_path,"w");
    if(file==NULL){
        fprintf(stderr, "Error opening file: %s\n", file_path);
        exit(1);
    }
    // Write header to the file
    fprintf(file, "edge_id;saved_cost\n");
    double_unsigned_list_t *current = head;
    while (current != NULL)
    {
        fprintf(file, "%u;%.4Lf\n", current->u_value,current->d_value);
        current = current->next;
    }
    fclose(file);
}

void *compute_optimize_for_budget_threaded(void *arg)
{
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    long double new_djikstra_cost, djikstra_cost, cost_difference;
    double *djikstra_backward_dist = (double *)calloc(thread_arg->nb_vertices, sizeof(double));
    double *djikstra_forward_dist = (double *)calloc(thread_arg->nb_vertices, sizeof(double));

    if (djikstra_backward_dist == NULL || djikstra_forward_dist == NULL)
    {
        return (void *)MEMORY_ALLOC_ERROR;
    }
    // by offsetting the loop index with the number of thread, we are sure that no two thread will ever
    // work on the same path
    for (uint32_t path_id = thread_arg->thread_id; (path_id < thread_arg->nb_paths); path_id += thread_arg->offset)
    {

        if (!thread_arg->impact[path_id])
        {
            continue;
        }
        thread_arg->impact[path_id] = false;
        djikstra_cost = djikstra_forward(thread_arg->graph, thread_arg->nb_vertices, &djikstra_forward_dist,NULL, thread_arg->paths[path_id]);
        djikstra_backward(thread_arg->graph, thread_arg->nb_vertices, &djikstra_backward_dist, NULL, thread_arg->paths[path_id]);

        for (uint32_t edge_id = 0; edge_id < thread_arg->nb_edges; edge_id++)
        {

            if (thread_arg->edge_array[edge_id]->dist > ((double)*thread_arg->budget_left))
            {
                continue;
            }
            // if the edge's vertexes are in the visibility of the path
            //  and the edge is not already optimized
            if (edge_is_in_visibilite(thread_arg->paths[path_id], thread_arg->edge_array[edge_id]) && (thread_arg->edge_array[edge_id]->dist != thread_arg->edge_array[edge_id]->danger))
            {

                new_djikstra_cost = updated_dist(thread_arg->edge_array[edge_id], thread_arg->paths[path_id], djikstra_forward_dist, djikstra_backward_dist);

                cost_difference = djikstra_cost - new_djikstra_cost;
                if (cost_difference > 0)
                {
                    // could be improved by creating a mutex for each edge
                    pthread_mutex_lock(thread_arg->mutex);
                    add_double_unsigned_list_t(&thread_arg->cost_diff_array[path_id], edge_id, cost_difference);
                    pthread_mutex_unlock(thread_arg->mutex);
                }
            }
        }
    }
    free(djikstra_backward_dist);
    free(djikstra_forward_dist);
    return (void *)OK;
}

int get_edges_to_optimize_for_budget_threaded(cifre_conf_t * config,long double *budget_used, double_unsigned_list_t **selected_edges)
{
    vertex_t **graph;
    path_t **paths;
    edge_t **edge_array;
    int ret_code = OK;
    (*selected_edges) = NULL;
    long double budget_left=config->budget;
    long double max_saved_cost;
    uint32_t nb_vertices, nb_edges, nb_paths;
    int32_t edge_id_to_optimize;
    bool stop = false;
    pthread_mutex_t mutex_cost_diff_array;
    thread_arg_t *thread_arg;
    pthread_t *threads;
    bool *impact;
    double_unsigned_list_t **cost_diff_array;

    pthread_mutex_init(&mutex_cost_diff_array, NULL);

    ret_code = get_graph(config, &graph, &edge_array, &nb_vertices, &nb_edges);
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
            .edge_array = edge_array,
            .graph = graph,
            .nb_edges = nb_edges,
            .nb_vertices = nb_vertices,
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
        // used to know the cost difference, the optimization of the edge would
        // bring for the path where the edge is in the visibility
        for (uint32_t i= 0; i < config->thread_number; i++)
        {
            pthread_create(&threads[i], NULL, compute_optimize_for_budget_threaded, (void *)&thread_arg[i]);
        }
        for (uint32_t i= 0; i < config->thread_number; i++)
        {
            // wait thread
            pthread_join(threads[i], NULL);
        }

        edge_id_to_optimize = -1;
        get_max_edge_to_optimize(cost_diff_array, nb_paths,nb_edges, edge_array, &edge_id_to_optimize, &max_saved_cost, budget_left);
        if (edge_id_to_optimize == -1)
        {
            stop = true;
        }
        else
        {
            // the next loop will only consider the paths who had the edge in their visibility
            for (uint32_t path_id = 0; path_id < nb_paths; path_id++)
            {
                if (edge_is_in_visibilite(paths[path_id], edge_array[edge_id_to_optimize]))
                {
                    impact[path_id] = true;
                    free_double_unsigned_list_t(cost_diff_array[path_id]);
                    cost_diff_array[path_id] = NULL;
                }
            }
            //add the selected edge to the selected_edge array
            ret_code = add_double_unsigned_list_t(selected_edges,edge_array[edge_id_to_optimize]->id, max_saved_cost);
            if (ret_code != OK)
            {
                free_double_unsigned_list_t(*selected_edges);
                *selected_edges = NULL;
                goto clean_up_cost_diff;
            }
            budget_left = budget_left - edge_array[edge_id_to_optimize]->dist;
            *budget_used = config->budget - budget_left;
            edge_array[edge_id_to_optimize]->danger = edge_array[edge_id_to_optimize]->dist;
        }
    }

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
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    return OK;
}

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
int get_edges_to_optimize_for_budget(cifre_conf_t * config, long double *budget_left, double_unsigned_list_t **selected_edges)
{
    vertex_t **graph;
    path_t **paths;
    edge_t **edge_array;
    int ret_code;
    (*selected_edges) = NULL;
    long double max_saved_cost;
    uint32_t nb_vertices, nb_edges, nb_paths;
    *budget_left = config->budget;
    int32_t edge_id_to_optimize;
    long double old_danger;
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

    djikstra_backward_dist = calloc(nb_vertices, sizeof(double));
    if (djikstra_backward_dist == NULL)
    {
        free_edge(edge_array, nb_edges);
        free_graph(graph, nb_vertices);
        free_paths(paths, nb_paths);
        fprintf(stderr, "Memory allocation failed for djikstra_backward_dist array\n");
        return MEMORY_ALLOC_ERROR;
    }
    djikstra_forward_dist = calloc(nb_vertices, sizeof(double));
    if (djikstra_forward_dist == NULL)
    {
        free_edge(edge_array, nb_edges);
        free_graph(graph, nb_vertices);
        free_paths(paths, nb_paths);
        free(djikstra_backward_dist);
        fprintf(stderr, "Memory allocation failed for djikstra_forward_dist array\n");
        return MEMORY_ALLOC_ERROR;
    }

    bool *impact = calloc(nb_paths, sizeof(bool));
    if (impact == NULL)
    {
        free_edge(edge_array, nb_edges);
        free_graph(graph, nb_vertices);
        free_paths(paths, nb_paths);
        free(djikstra_forward_dist);
        free(djikstra_backward_dist);
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
        free(djikstra_forward_dist);
        free(djikstra_backward_dist);
        fprintf(stderr, "Memory allocation failed for cost_diff_array array\n");
        return MEMORY_ALLOC_ERROR;
    }
    while (!stop)
    {
        // used to know the cost difference, the optimization of the edge would
        // bring for the path where the edge is in the visibility

        for (uint32_t path_id = 0; (path_id < nb_paths); path_id++)
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
                if (edge_array[edge_id]->dist > *budget_left)
                {
                    continue;
                }
                //  if the edge's vertexes are in the visibility of the path
                //  and the edge is not already optimized
                if (edge_is_in_visibilite(paths[path_id], edge_array[edge_id]) && (edge_array[edge_id]->dist != edge_array[edge_id]->danger))
                {

                    // optimizing an edge is making its danger equal to its distance
                    old_danger = edge_array[edge_id]->danger;
                    edge_array[edge_id]->danger = edge_array[edge_id]->dist;

                    new_djikstra_cost = updated_dist(edge_array[edge_id], paths[path_id], djikstra_forward_dist, djikstra_backward_dist);

                    // unoptimize the edge
                    edge_array[edge_id]->danger = old_danger;

                    cost_difference = djikstra_cost - new_djikstra_cost;
                    if (cost_difference > 0)
                    {
                        add_double_unsigned_list_t(&cost_diff_array[path_id], edge_id, cost_difference);

                    }
                }
            }
        }
        edge_id_to_optimize = -1;
        get_max_edge_to_optimize(cost_diff_array, nb_paths,nb_edges, edge_array, &edge_id_to_optimize, &max_saved_cost, *budget_left);
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
            ret_code = add_double_unsigned_list_t(selected_edges,edge_array[edge_id_to_optimize]->id, max_saved_cost);
            if (ret_code != OK)
            {
                free(impact);
                free(djikstra_backward_dist);
                free(djikstra_forward_dist);
                free_edge(edge_array, nb_edges);
                free_graph(graph, nb_vertices);
                free_paths(paths, nb_paths);
                free_double_unsigned_list_t(*selected_edges);
                return ret_code;
            }
            *budget_left = *budget_left - edge_array[edge_id_to_optimize]->dist;
            edge_array[edge_id_to_optimize]->danger = edge_array[edge_id_to_optimize]->dist;
        }
    }
    free(impact);
    free(djikstra_backward_dist);
    free(djikstra_forward_dist);
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_paths(paths, nb_paths);
    free_cost_diff_array(cost_diff_array,nb_paths);
    return OK;
}

int init_cost_diff_array(double_unsigned_list_t ***diff_array, unsigned int nb_paths)
{
    *diff_array = calloc(nb_paths, sizeof(double_unsigned_list_t *)); 
    if(*diff_array == NULL){
        return MEMORY_ALLOC_ERROR;
    }
    for (unsigned int i = 0; i < nb_paths; i++)
    {
        (*diff_array)[i] = NULL;
    }
    return OK;
}

void free_cost_diff_array(double_unsigned_list_t **array, uint32_t nb_paths){
    for (size_t path_id = 0; path_id < nb_paths; path_id++)
    {
        free_double_unsigned_list_t(array[path_id]);
    }
    free(array);
}

void get_max_edge_to_optimize(double_unsigned_list_t **diff_array,uint32_t nb_paths, uint32_t nb_edges, edge_t **edge_array, int *edge_id_to_optimize, long double *saved_cost, long double budget_left)
{
    long double max_cost_saved = 0;
    uint32_t max_cost_edge_id;
    long double *edge_cost_saved = calloc(nb_edges,sizeof(long double));
    double_unsigned_list_t * temp;

    for (uint32_t path_id = 0; path_id < nb_paths; path_id++)
    {
        temp = diff_array[path_id];
        while(temp!=NULL){
            edge_cost_saved[temp->u_value]+=temp->d_value;
            temp = temp->next;
        }
    }
    
    for (uint32_t id_edge = 0; id_edge < nb_edges; id_edge++)
    {

        if ((edge_array[id_edge]->dist <= (budget_left)) && edge_cost_saved[id_edge] > max_cost_saved)
        {
            max_cost_saved = edge_cost_saved[id_edge];
            max_cost_edge_id = id_edge;
        }
    }
    if (max_cost_saved > 0)
    {
        *saved_cost = max_cost_saved;
        *edge_id_to_optimize = max_cost_edge_id;
    }
    free(edge_cost_saved);
}