#include "../header/djikstra.h"
#include <stdbool.h>

void get_edges_to_optimize_for_budget(long double budget)
{
    vertex_t *graph;
    path_t *paths;
    edge_t **edge_array;
    int nb_vertices, nb_edges, nb_paths;
    long double budget_left = budget;
    int edge_id_to_optimize;
    long double old_danger;
    long double new_djikstra_cost,djikstra_cost, cost_difference;
    bool stop = false;

    get_graph("./data/artificiel/data_graphe.csv", ";", &graph, &edge_array, &nb_vertices, &nb_edges);
    get_paths("./data/artificiel/data_path.csv", ";", &paths, &nb_paths);

    long double cost_diff_array[nb_edges];

    while (!stop)
    {
        // used to know the cost difference, the optimization of the edge would 
        // bring for the path where the edge is in the visibility
        init_cost_diff_array(cost_diff_array, nb_edges);

        for (int path_id = 0; path_id < nb_paths; path_id++)
        {
            djikstra_cost = djikstra_backward(graph, nb_vertices, &paths[path_id].backward_djikstra, NULL, &paths[path_id]);
            djikstra_forward(graph, nb_vertices, &paths[path_id].foward_djikstra, NULL, &paths[path_id]);

            for (int edge_id = 0; edge_id < nb_edges; edge_id++)
            {
                // if the edge's vertexes are in the visibility of the path
                //  and the edge is not already optimized
                if (edge_is_in_visibilite(&paths[path_id], edge_array[edge_id]) && (edge_array[edge_id]->dist != edge_array[edge_id]->danger))
                {

                    // optimizing an edge is making its danger equal to its distance
                    old_danger = edge_array[edge_id]->danger;
                    edge_array[edge_id]->danger = edge_array[edge_id]->dist;

                    new_djikstra_cost = updated_dist(edge_array[edge_id], &paths[path_id]);

                    // unoptimize the edge
                    edge_array[edge_id]->danger = old_danger;

                    cost_difference = djikstra_cost - new_djikstra_cost;
                    if (cost_difference > 0)
                    {
                        cost_diff_array[edge_id] += cost_difference;
                    }
                }
            }
            free(paths[path_id].backward_djikstra);
            free(paths[path_id].foward_djikstra);
        }
        edge_id_to_optimize = -1;
        get_max_edge_to_optimize(cost_diff_array, nb_edges, edge_array, &edge_id_to_optimize, budget_left);
        if (edge_id_to_optimize == -1)
        {
            stop = true;
        }
        else
        {
            budget_left = budget_left - edge_array[edge_id_to_optimize]->dist;
            edge_array[edge_id_to_optimize]->danger = edge_array[edge_id_to_optimize]->dist;
            printf(" %f\n", edge_array[edge_id_to_optimize]->dist);
        }
    }
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

void get_max_edge_to_optimize(long double *diff_array, unsigned int nb_edges,edge_t ** edge_array, int *edge_id_to_optimize, long double budget_left)
{
    long double max_cost_saved = 0;
    unsigned int max_cost_edge_id;
    // cost_diff_edge_t *temp;

    for (unsigned int id_edge = 0; id_edge < nb_edges; id_edge++)
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
        *edge_id_to_optimize = max_cost_edge_id;
        printf("%d %Lf", max_cost_edge_id, max_cost_saved);
    }
}

double cost_function(path_t *path, edge_t *edge)
{
    return (path->profil * edge->dist) + ((1 - path->profil) * edge->danger);
    // return edge->dist;
}

/// update the distance in the cached djikstra
/// here the vertex already has an updated distance
double updated_dist(edge_t *edge, path_t *path)
{
    return path->foward_djikstra[edge->pred->id] + cost_function(path, edge) + path->backward_djikstra[edge->succ->id];
}

double djikstra_forward(struct vertex_t *graph, int V, double **dist_array,int** parent_array, path_t *path)
{

    (*dist_array) = (double *)calloc(1,V * sizeof(double));
    int * parent;
    bool markedVertex[V];
    bool toVisitVertex[V];
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;

    if(parent_array!=NULL){
        (*parent_array) = (int *)calloc(1,V * sizeof(int));
        parent = (*parent_array);
    }else{
        parent = (int *)calloc(1,V * sizeof(int));
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

    verticesToVisit = calloc(1,sizeof(list_node_t));
    verticesToVisit->vertex_id = origin;
    verticesToVisit->next = NULL;
    for (unsigned int i = 0; i < graph[origin].nb_edges_out; i++)
    {
        dest_vertex_id = graph[origin].out_edges[i]->succ->id;

        list_node_t *newNode = calloc(1,sizeof(list_node_t));
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
            if (!markedVertex[dest_vertex_id] && graph[u].out_edges[edge_i] != NULL && ((*dist_array)[u] + cost_function(path, graph[u].out_edges[edge_i]) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(path, graph[u].out_edges[edge_i]);
                parent[dest_vertex_id] = u;
                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    list_node_t *vertexToVisit = calloc(1,sizeof(list_node_t));
                    vertexToVisit->next = verticesToVisit;
                    vertexToVisit->vertex_id = dest_vertex_id;
                    verticesToVisit = vertexToVisit;
                }
            }
        }
    }
    temp = verticesToVisit;
    while(temp!=NULL){
        old = temp;
        temp = temp->next;
        free(old);
    }
    if(parent_array==NULL){
        free(parent);
    }
    return (*dist_array)[destination];
}

double djikstra_backward(struct vertex_t *graph, int V, double **dist_array,int **parent_array, path_t *path)
{
    (*dist_array) = (double *)calloc(1,V * sizeof(double));
    bool markedVertex[V];
    bool toVisitVertex[V];
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;
    int *parent;

    if(parent_array!=NULL){
        (*parent_array) = (int *)calloc(1,V * sizeof(int));
        parent = (*parent_array);
    }else{
        parent = (int *)calloc(1,V * sizeof(int));
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


    verticesToVisit = calloc(1,sizeof(list_node_t));
    verticesToVisit->vertex_id = origin;
    verticesToVisit->next = NULL;
    for (unsigned int i = 0; i < graph[origin].nb_edges_in; i++)
    {
        dest_vertex_id = graph[origin].in_edges[i]->pred->id;

        list_node_t *newNode = calloc(1,sizeof(list_node_t));
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
            if (!markedVertex[dest_vertex_id] && graph[u].in_edges[edge_i] != NULL && ((*dist_array)[u] + cost_function(path, graph[u].in_edges[edge_i]) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(path, graph[u].in_edges[edge_i]);
                parent[dest_vertex_id] = u;
                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    list_node_t *vertexToVisit = calloc(1,sizeof(list_node_t));
                    vertexToVisit->next = verticesToVisit;
                    vertexToVisit->vertex_id = dest_vertex_id;
                    verticesToVisit = vertexToVisit;
                }
            }
        }
    }

    //free the remaining vertices
    temp = verticesToVisit;
    while(temp!=NULL){
        old = temp;
        temp = temp->next;
        free(old);
    }
    // if parent is not needed
    if(parent_array==NULL){
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
// TODO : REMOVE PARENTS
//  Dijkstra's algorithm to find the shortest path between two vertices
double djikstra_test(struct vertex_t *graph, int V, double **dist_array, int **parent, path_t *path)
{
    (*dist_array) = (double *)calloc(1,V * sizeof(double));
    bool markedVertex[V];
    bool toVisitVertex[V];
    (*parent) = (int *)calloc(1,V * sizeof(int));
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;

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

    // Parent of source is itself
    (*parent)[origin] = -1;

    verticesToVisit = calloc(1,sizeof(list_node_t));
    verticesToVisit->vertex_id = origin;
    verticesToVisit->next = NULL;
    for (unsigned int i = 0; i < graph[origin].nb_edges_out; i++)
    {
        dest_vertex_id = graph[origin].out_edges[i]->succ->id;

        list_node_t *newNode = calloc(1,sizeof(list_node_t));
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
            if (!markedVertex[dest_vertex_id] && graph[u].out_edges[edge_i] != NULL && ((*dist_array)[u] + cost_function(path, graph[u].out_edges[edge_i]) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(path, graph[u].out_edges[edge_i]);
                (*parent)[dest_vertex_id] = u;

                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    list_node_t *vertexToVisit = calloc(1,sizeof(list_node_t));
                    vertexToVisit->next = verticesToVisit;
                    vertexToVisit->vertex_id = dest_vertex_id;
                    verticesToVisit = vertexToVisit;
                }
            }
        }
    }

    return (*dist_array)[destination];
}