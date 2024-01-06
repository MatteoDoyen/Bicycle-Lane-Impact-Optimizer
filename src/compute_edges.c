#include <stdbool.h>
#include <stdlib.h>
#include "../header/util.h"
#include "../header/compute_edges.h"

void print_selected_edges(selected_edge_t *head)
{
    selected_edge_t *current = head;
    while (current != NULL)
    {
        printf("%d %Lf\n", current->edge_id, current->cost_saved);
        current = current->next;
    }
}

void free_select_edges(selected_edge_t *head)
{
    selected_edge_t *current = head;
    selected_edge_t *next_edge;

    while (current != NULL)
    {
        next_edge = current->next;
        free(current);
        current = next_edge;
    }
}

int new_selected_edge(unsigned edge_id, long double cost_saved, selected_edge_t **head)
{
    selected_edge_t *new_edge = (selected_edge_t *)calloc(1, sizeof(selected_edge_t));
    if (new_edge == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return MEMORY_ALLOC_ERROR;
    }

    new_edge->cost_saved = cost_saved;
    new_edge->edge_id = edge_id;
    new_edge->next = *head;
    *head = new_edge;
    return OK;
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

    int *parents_forward;
    // by offsetting the loop index with the number of thread, we are sure that no two thread will ever
    // work on the same path
    for (uint32_t path_id = thread_arg->thread_id; (path_id < thread_arg->nb_paths); path_id += thread_arg->offset)
    {

        if (!thread_arg->impact[path_id])
        {
            continue;
        }
        thread_arg->impact[path_id] = false;
        djikstra_cost = djikstra_forward(thread_arg->graph, thread_arg->nb_vertices, &djikstra_forward_dist, &parents_forward, thread_arg->paths[path_id]);
        djikstra_backward(thread_arg->graph, thread_arg->nb_vertices, &djikstra_backward_dist, NULL, thread_arg->paths[path_id]);
        free(parents_forward);
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
                    thread_arg->cost_diff_array[edge_id] += cost_difference;
                    pthread_mutex_unlock(thread_arg->mutex);
                }
            }
        }
    }
    free(djikstra_backward_dist);
    free(djikstra_forward_dist);
    return (void *)OK;
}

int get_edges_to_optimize_for_budget_threaded(long double budget, char *graphe_file_name, char *paths_file_name, int nb_thread, selected_edge_t **selected_edges)
{
    vertex_t **graph;
    path_t **paths;
    edge_t **edge_array;
    int ret_code;
    (*selected_edges) = NULL;
    long double max_saved_cost;
    uint32_t nb_vertices, nb_edges, nb_paths;
    long double budget_left = budget;
    int32_t edge_id_to_optimize;
    bool stop = false;
    pthread_mutex_t mutex_cost_diff_array;

    pthread_mutex_init(&mutex_cost_diff_array, NULL);

    ret_code = get_graph(graphe_file_name, ";", &graph, &edge_array, &nb_vertices, &nb_edges);
    if (ret_code != OK)
    {
        return ret_code;
    }
    // fprintf(stderr,"before path\n");
    ret_code = get_paths(paths_file_name, ";", &paths, &nb_paths);
    if (ret_code != OK)
    {
        free_edge(edge_array, nb_edges);
        free_graph(graph, nb_vertices);
        return ret_code;
    }
    // fprintf(stderr, "start algo\n");
    bool *impact = calloc(nb_paths, sizeof(bool));
    if (impact == NULL)
    {
        free_edge(edge_array, nb_edges);
        free_graph(graph, nb_vertices);
        free_paths(paths, nb_paths);
        fprintf(stderr, "Memory allocation failed for impact array\n");
        return MEMORY_ALLOC_ERROR;
    }
    long double *cost_diff_array = calloc(nb_edges, sizeof(long double));
    if (cost_diff_array == NULL)
    {
        free(impact);
        free_edge(edge_array, nb_edges);
        free_graph(graph, nb_vertices);
        free_paths(paths, nb_paths);
        fprintf(stderr, "Memory allocation failed for cost_diff_array\n");
        return MEMORY_ALLOC_ERROR;
    }

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        impact[i] = true;
    }
    thread_arg_t thread_arg[nb_thread];
    pthread_t threads[nb_thread];
    for (int i = 0; i < nb_thread; i++)
    {
        thread_arg[i].cost_diff_array = cost_diff_array;
        thread_arg[i].edge_array = edge_array;
        thread_arg[i].graph = graph;
        thread_arg[i].nb_edges = nb_edges;
        thread_arg[i].nb_vertices = nb_vertices;
        thread_arg[i].nb_paths = nb_paths;
        thread_arg[i].thread_id = i;
        thread_arg[i].offset = nb_thread;
        thread_arg[i].mutex = &mutex_cost_diff_array;
        thread_arg[i].paths = paths;
        thread_arg[i].impact = impact;
        thread_arg[i].budget_left = &budget_left;
    }

    while (!stop)
    {
        // used to know the cost difference, the optimization of the edge would
        // bring for the path where the edge is in the visibility
        init_cost_diff_array(cost_diff_array, nb_edges);
        for (int i = 0; i < nb_thread; i++)
        {
            pthread_create(&threads[i], NULL, compute_optimize_for_budget_threaded, (void *)&thread_arg[i]);
        }
        for (int i = 0; i < nb_thread; i++)
        {
            // wait thread
            pthread_join(threads[i], NULL);
        }

        edge_id_to_optimize = -1;
        get_max_edge_to_optimize(cost_diff_array, nb_edges, edge_array, &edge_id_to_optimize, &max_saved_cost, budget_left);
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
                }
            }
            ret_code = new_selected_edge(edge_array[edge_id_to_optimize]->id, max_saved_cost, selected_edges);
            if (ret_code != OK)
            {
                free(cost_diff_array);
                free(impact);
                free_edge(edge_array, nb_edges);
                free_graph(graph, nb_vertices);
                free_paths(paths, nb_paths);
                free_select_edges(*selected_edges);
                return ret_code;
            }
            budget_left = budget_left - edge_array[edge_id_to_optimize]->dist;
            edge_array[edge_id_to_optimize]->danger = edge_array[edge_id_to_optimize]->dist;
        }
    }
    free(cost_diff_array);
    free(impact);
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_paths(paths, nb_paths);
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
   edge within their visibility and compare the new "cost" to the old cost of
   the paths.
   If the costs is smaller, it is added to the total costs saved by improving
   the edge.
   If the improvement of the road had a non-null impact the graph is then
   updated to take in this improvement. The function stops when the budget
   is reached or that no other edges improvement has an impact

   --------------------------------------------------------------------------
   Return value:
   void

-- -------------------------------------------------------------------------- */
int get_edges_to_optimize_for_budget(long double budget, char *graphe_file_name, char *paths_file_name, selected_edge_t **selected_edges)
{
    vertex_t **graph;
    path_t **paths;
    edge_t **edge_array;
    int ret_code;
    (*selected_edges) = NULL;
    long double max_saved_cost;
    uint32_t nb_vertices, nb_edges, nb_paths;
    long double budget_left = budget;
    int32_t edge_id_to_optimize;
    long double old_danger;
    double *djikstra_backward_dist, *djikstra_forward_dist;
    long double new_djikstra_cost, djikstra_cost, cost_difference;
    bool stop = false;

    ret_code = get_graph(graphe_file_name, ";", &graph, &edge_array, &nb_vertices, &nb_edges);
    if (ret_code != OK)
    {
        return ret_code;
    }
    // fprintf(stderr,"Available stack size is %d bytes\n", stackavail());
    ret_code = get_paths(paths_file_name, ";", &paths, &nb_paths);
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

    long double *cost_diff_array = calloc(nb_edges, sizeof(long double));
    if (cost_diff_array == NULL)
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
    fprintf(stderr, "debut algo \n");
    while (!stop)
    {
        // used to know the cost difference, the optimization of the edge would
        // bring for the path where the edge is in the visibility
        init_cost_diff_array(cost_diff_array, nb_edges);

        for (uint32_t path_id = 0; (path_id < nb_paths); path_id++)
        {
            if (!impact[path_id])
            {
                // printf("in\n");
                continue;
            }
            impact[path_id] = false;

            //  calculating the djikstra path backward and forward allows for
            //  a much faster calculating of the impact of the improvement of an edge
            djikstra_cost = djikstra_backward(graph, nb_vertices, &djikstra_backward_dist, NULL, paths[path_id]);
            djikstra_forward(graph, nb_vertices, &djikstra_forward_dist, NULL, paths[path_id]);
            // fprintf(stderr, "djikstra done\n");

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

                    // optimizing an edge is making its danger equal to its distance
                    old_danger = edge_array[edge_id]->danger;
                    edge_array[edge_id]->danger = edge_array[edge_id]->dist;

                    new_djikstra_cost = updated_dist(edge_array[edge_id], paths[path_id], djikstra_forward_dist, djikstra_backward_dist);

                    // unoptimize the edge
                    edge_array[edge_id]->danger = old_danger;

                    cost_difference = djikstra_cost - new_djikstra_cost;
                    if (cost_difference > 0)
                    {
                        // fprintf(stderr,"meilleur chemin trouvéé\n");
                        cost_diff_array[edge_id] = cost_difference + cost_diff_array[edge_id];
                    }
                }
            }

            // fprintf(stderr, "b free dist\n");
            // fprintf(stderr, "freed dist\n");
        }
        edge_id_to_optimize = -1;
        get_max_edge_to_optimize(cost_diff_array, nb_edges, edge_array, &edge_id_to_optimize, &max_saved_cost, budget_left);
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
                }
            }
            ret_code = new_selected_edge(edge_array[edge_id_to_optimize]->id, max_saved_cost, selected_edges);
            if (ret_code != OK)
            {
                free(impact);
                free(djikstra_backward_dist);
                free(djikstra_forward_dist);
                free_edge(edge_array, nb_edges);
                free_graph(graph, nb_vertices);
                free_paths(paths, nb_paths);
                free_select_edges(*selected_edges);
                return ret_code;
            }
            budget_left = budget_left - edge_array[edge_id_to_optimize]->dist;
            // fprintf(stderr,"budget restant %Lf\n",budget_left);
            edge_array[edge_id_to_optimize]->danger = edge_array[edge_id_to_optimize]->dist;
        }
    }
    // fprintf(stderr, "??\n");
    free(impact);
    free(djikstra_backward_dist);
    free(djikstra_forward_dist);
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_paths(paths, nb_paths);
    return OK;
}

void init_cost_diff_array(long double *diff_array, unsigned int nb_edges)
{
    for (unsigned int i = 0; i < nb_edges; i++)
    {
        diff_array[i] = 0;
    }
}

void get_max_edge_to_optimize(long double *diff_array, uint32_t nb_edges, edge_t **edge_array, int *edge_id_to_optimize, long double *saved_cost, long double budget_left)
{
    long double max_cost_saved = 0;
    uint32_t max_cost_edge_id;
    // cost_diff_edge_t *temp;

    for (uint32_t id_edge = 0; id_edge < nb_edges; id_edge++)
    {

        if ((edge_array[id_edge]->dist <= (budget_left)) && diff_array[id_edge] > max_cost_saved)
        {
            max_cost_saved = diff_array[id_edge];
            max_cost_edge_id = id_edge;
        }
    }
    if (max_cost_saved > 0)
    {
        *saved_cost = max_cost_saved;
        *edge_id_to_optimize = max_cost_edge_id;
    }
}