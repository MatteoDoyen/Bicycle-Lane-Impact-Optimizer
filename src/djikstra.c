#include "../header/djikstra.h"
#include <stdbool.h>

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

void new_selected_edge(unsigned edge_id, long double cost_saved, selected_edge_t **head)
{
    selected_edge_t *new_edge = (struct selected_edge_t *)calloc(1, sizeof(struct selected_edge_t));
    if (new_edge == NULL)
    {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    new_edge->cost_saved = cost_saved;
    new_edge->edge_id = edge_id;
    new_edge->next = *head;
    *head = new_edge;
}

void *compute_optimize_for_budget_threaded(void *arg)
{
    thread_arg_t *thread_arg = (thread_arg_t *)arg;
    long double new_djikstra_cost, djikstra_cost, cost_difference;
    double *djikstra_backward_dist;
    double *djikstra_forward_dist;
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
        djikstra_cost = djikstra_forward(thread_arg->graph, thread_arg->nb_vertices, &djikstra_forward_dist, &parents_forward, &thread_arg->paths[path_id]);
        djikstra_backward(thread_arg->graph, thread_arg->nb_vertices, &djikstra_backward_dist, NULL, &thread_arg->paths[path_id]);
        free(parents_forward);
        // dijistkra_backward_2(thread_arg->nb_vertices, djikstra_forward_dist, &djikstra_backward_dist, parents_forward, djikstra_cost, &thread_arg->paths[path_id]);
        for (uint32_t edge_id = 0; edge_id < thread_arg->nb_edges; edge_id++)
        {
            // if the edge's vertexes are in the visibility of the path
            //  and the edge is not already optimized
            if (edge_is_in_visibilite(&thread_arg->paths[path_id], thread_arg->edge_array[edge_id]) && (thread_arg->edge_array[edge_id]->dist != thread_arg->edge_array[edge_id]->danger))
            {

                new_djikstra_cost = updated_dist(thread_arg->edge_array[edge_id], &thread_arg->paths[path_id], djikstra_forward_dist, djikstra_backward_dist);

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
        free(djikstra_backward_dist);
        free(djikstra_forward_dist);
    }
    return NULL;
}

void get_edges_to_optimize_for_budget_threaded(long double budget, char *graphe_file_name, char *paths_file_name, int nb_thread, selected_edge_t **selected_edges)
{
    vertex_t *graph;
    path_t *paths;
    edge_t **edge_array;
    (*selected_edges) = NULL;
    long double max_saved_cost;
    uint32_t nb_vertices, nb_edges, nb_paths;
    long double budget_left = budget;
    int32_t edge_id_to_optimize;
    bool stop = false;
    pthread_mutex_t mutex_cost_diff_array;

    pthread_mutex_init(&mutex_cost_diff_array, NULL);

    get_graph(graphe_file_name, ";", &graph, &edge_array, &nb_vertices, &nb_edges);
    // fprintf(stderr,"before path\n");
    get_paths(paths_file_name, ";", &paths, &nb_paths);
    // fprintf(stderr,"after\n");
    bool *impact = calloc(1, sizeof(bool) * nb_paths);
    for (uint32_t i = 0; i < nb_paths; i++)
    {
        impact[i] = true;
    }

    long double cost_diff_array[nb_edges];
    thread_arg_t thread_arg[nb_edges];
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
                if (edge_is_in_visibilite(&paths[path_id], edge_array[edge_id_to_optimize]))
                {
                    impact[path_id] = true;
                }
            }
            new_selected_edge(edge_array[edge_id_to_optimize]->id, max_saved_cost, selected_edges);
            budget_left = budget_left - edge_array[edge_id_to_optimize]->dist;
            edge_array[edge_id_to_optimize]->danger = edge_array[edge_id_to_optimize]->dist;
        }
    }
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_paths(paths, nb_paths);
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
void get_edges_to_optimize_for_budget(long double budget, char *graphe_file_name, char *paths_file_name, selected_edge_t **selected_edges)
{
    vertex_t *graph;
    path_t *paths;
    edge_t **edge_array;
    (*selected_edges) = NULL;
    long double max_saved_cost;
    uint32_t nb_vertices, nb_edges, nb_paths;
    long double budget_left = budget;
    int32_t edge_id_to_optimize;
    long double old_danger;
    double *djikstra_backward_dist, *djikstra_forward_dist;
    long double new_djikstra_cost, djikstra_cost, cost_difference;
    bool stop = false;

    get_graph(graphe_file_name, ";", &graph, &edge_array, &nb_vertices, &nb_edges);
    get_paths(paths_file_name, ";", &paths, &nb_paths);

    bool impact[nb_paths];

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        impact[i] = true;
    }

    long double cost_diff_array[nb_edges];

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
            djikstra_cost = djikstra_backward(graph, nb_vertices, &djikstra_backward_dist, NULL, &paths[path_id]);
            djikstra_forward(graph, nb_vertices, &djikstra_forward_dist, NULL, &paths[path_id]);
            // fprintf(stderr, "djikstra done\n");

            for (uint32_t edge_id = 0; edge_id < nb_edges; edge_id++)
            {

                //  if the edge's vertexes are in the visibility of the path
                //  and the edge is not already optimized
                if (edge_is_in_visibilite(&paths[path_id], edge_array[edge_id]) && (edge_array[edge_id]->dist != edge_array[edge_id]->danger))
                {

                    // optimizing an edge is making its danger equal to its distance
                    old_danger = edge_array[edge_id]->danger;
                    edge_array[edge_id]->danger = edge_array[edge_id]->dist;

                    new_djikstra_cost = updated_dist(edge_array[edge_id], &paths[path_id], djikstra_forward_dist, djikstra_backward_dist);

                    // unoptimize the edge
                    edge_array[edge_id]->danger = old_danger;

                    cost_difference = djikstra_cost - new_djikstra_cost;
                    if (cost_difference > 0)
                    {
                        cost_diff_array[edge_id] = cost_difference + cost_diff_array[edge_id];
                    }
                }
            }

            // fprintf(stderr, "b free dist\n");
            free(djikstra_backward_dist);
            free(djikstra_forward_dist);
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
                if (edge_is_in_visibilite(&paths[path_id], edge_array[edge_id_to_optimize]))
                {
                    impact[path_id] = true;
                }
            }
            new_selected_edge(edge_array[edge_id_to_optimize]->id, max_saved_cost, selected_edges);
            budget_left = budget_left - edge_array[edge_id_to_optimize]->dist;
            edge_array[edge_id_to_optimize]->danger = edge_array[edge_id_to_optimize]->dist;
        }
    }
    // fprintf(stderr, "??\n");
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_paths(paths, nb_paths);
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
            // printf("diff %f budget %Lf\n",edge_array[id_edge]->dist,budget_left);
        }
    }
    if (max_cost_saved > 0)
    {
        *saved_cost = max_cost_saved;
        *edge_id_to_optimize = max_cost_edge_id;
    }
}

double cost_function(double *alpha, double *distance, double *danger)
{
    return ((*alpha) * (*distance)) + ((1 - (*alpha)) * (*danger));
}

/// update the distance in the cached djikstra
/// here the vertex already has an updated distance
double updated_dist(edge_t *edge, path_t *path, double *foward_djikstra, double *backward_djikstra)
{
    return foward_djikstra[edge->pred->id] + cost_function(&path->profil, &edge->dist, &edge->dist) + backward_djikstra[edge->succ->id];
}

double djikstra_forward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, path_t *path)
{
    (*dist_array) = (double *)calloc(1, V * sizeof(double));
    int *parent;
    bool markedVertex[V];
    bool toVisitVertex[V];
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;

    if (parent_array != NULL)
    {
        (*parent_array) = (int *)calloc(1, V * sizeof(int));
        parent = (*parent_array);
    }
    else
    {
        parent = calloc(1, V * sizeof(int));
    }
    origin = path->origin;
    destination = path->destination;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < V; i++)
    {
        (*dist_array)[i] = DBL_MAX;
        markedVertex[i] = false;
        toVisitVertex[i] = false;
    }

    // Distance from source to itself is always 0
    (*dist_array)[origin] = 0;

    parent[origin] = -1;

    verticesToVisit = calloc(1, sizeof(list_node_t));
    verticesToVisit->vertex_id = origin;
    verticesToVisit->next = NULL;
    for (unsigned int i = 0; i < graph[origin].nb_edges_out; i++)
    {
        dest_vertex_id = graph[origin].out_edges[i]->succ->id;

        list_node_t *newNode = calloc(1, sizeof(list_node_t));
        toVisitVertex[dest_vertex_id] = true;
        newNode->vertex_id = dest_vertex_id;
        newNode->next = verticesToVisit;
        verticesToVisit = newNode;
    }
    // Find the shortest path for all vertices
    while (verticesToVisit != NULL)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        int u = min_distance((*dist_array), verticesToVisit);
        if (u == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[u] = true;

        temp = verticesToVisit;
        old = NULL;
        while (temp != NULL && temp->vertex_id != u)
        {
            old = temp;
            temp = temp->next;
        }
        if (old != NULL)
        {
            old->next = temp->next;
        }
        else
        {
            verticesToVisit = temp->next;
        }
        free(temp);

        // Update dist value of the adjacent vertices
        for (unsigned int edge_i = 0; edge_i < graph[u].nb_edges_out; edge_i++)
        {
            dest_vertex_id = graph[u].out_edges[edge_i]->succ->id;
            if (!vertex_is_in_visibilite(path, dest_vertex_id))
            {
                continue;
            }
            if (!markedVertex[dest_vertex_id] && graph[u].out_edges[edge_i] != NULL && ((*dist_array)[u] + cost_function(&path->profil, &graph[u].out_edges[edge_i]->dist, &graph[u].out_edges[edge_i]->danger) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(&path->profil, &graph[u].out_edges[edge_i]->dist, &graph[u].out_edges[edge_i]->danger);
                parent[dest_vertex_id] = u;
                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    list_node_t *vertexToVisit = calloc(1, sizeof(list_node_t));
                    vertexToVisit->next = verticesToVisit;
                    vertexToVisit->vertex_id = dest_vertex_id;
                    verticesToVisit = vertexToVisit;
                }
            }
        }
    }
    temp = verticesToVisit;
    while (temp != NULL)
    {
        old = temp;
        temp = temp->next;
        free(old);
    }
    if (parent_array == NULL)
    {
        free(parent);
    }
    return (*dist_array)[destination];
}

double djikstra_backward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, path_t *path)
{
    (*dist_array) = (double *)calloc(1, V * sizeof(double));
    bool markedVertex[V];
    bool toVisitVertex[V];
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;
    int *parent;

    if (parent_array != NULL)
    {
        (*parent_array) = (int *)calloc(1, V * sizeof(int));
        parent = (*parent_array);
    }
    else
    {
        parent = (int *)calloc(1, V * sizeof(int));
    }

    origin = path->destination;
    destination = path->origin;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < V; i++)
    {
        (*dist_array)[i] = DBL_MAX;
        markedVertex[i] = false;
        toVisitVertex[i] = false;
    }

    // Distance from source to itself is always 0
    (*dist_array)[origin] = 0;

    parent[origin] = -1;

    verticesToVisit = calloc(1, sizeof(list_node_t));
    verticesToVisit->vertex_id = origin;
    verticesToVisit->next = NULL;
    for (unsigned int i = 0; i < graph[origin].nb_edges_in; i++)
    {
        dest_vertex_id = graph[origin].in_edges[i]->pred->id;

        list_node_t *newNode = calloc(1, sizeof(list_node_t));
        toVisitVertex[dest_vertex_id] = true;
        newNode->vertex_id = dest_vertex_id;
        newNode->next = verticesToVisit;
        verticesToVisit = newNode;
    }

    // Find the shortest path for all vertices
    while (verticesToVisit != NULL)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        int u = min_distance((*dist_array), verticesToVisit);
        if (u == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[u] = true;

        temp = verticesToVisit;
        old = NULL;
        while (temp != NULL && temp->vertex_id != u)
        {
            old = temp;
            temp = temp->next;
        }
        if (old != NULL)
        {
            old->next = temp->next;
        }
        else
        {
            verticesToVisit = temp->next;
        }
        free(temp);

        // Update dist value of the adjacent vertices
        for (unsigned int edge_i = 0; edge_i < graph[u].nb_edges_in; edge_i++)
        {
            dest_vertex_id = graph[u].in_edges[edge_i]->pred->id;
            if (!vertex_is_in_visibilite(path, dest_vertex_id))
            {
                continue;
            }
            if (!markedVertex[dest_vertex_id] && graph[u].in_edges[edge_i] != NULL && ((*dist_array)[u] + cost_function(&path->profil, &graph[u].in_edges[edge_i]->dist, &graph[u].in_edges[edge_i]->danger) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(&path->profil, &graph[u].in_edges[edge_i]->dist, &graph[u].in_edges[edge_i]->danger);
                parent[dest_vertex_id] = u;
                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    list_node_t *vertexToVisit = calloc(1, sizeof(list_node_t));
                    vertexToVisit->next = verticesToVisit;
                    vertexToVisit->vertex_id = dest_vertex_id;
                    verticesToVisit = vertexToVisit;
                }
            }
        }
    }

    // free the remaining vertices
    temp = verticesToVisit;
    while (temp != NULL)
    {
        old = temp;
        temp = temp->next;
        free(old);
    }
    // if parent is not needed
    if (parent_array == NULL)
    {
        free(parent);
    }

    return (*dist_array)[destination];
}

int min_distance(double dist[], list_node_t *vertexToVisit)
{
    double min = DBL_MAX;
    int min_index = -1;

    list_node_t *temp = vertexToVisit;

    while (temp != NULL)
    {
        if (dist[temp->vertex_id] < min)
        {
            min = dist[temp->vertex_id];
            min_index = temp->vertex_id;
        }
        temp = temp->next;
    }
    return min_index;
}