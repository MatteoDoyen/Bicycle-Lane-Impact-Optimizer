#include "../header/djikstra.h"
#include <stdbool.h>
#include <stdlib.h>
#include "../header/util.h"


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

double djikstra_forward(struct vertex_t **graph, int V, double **dist_array, int **parent_array, path_t *path)
{
    int *parent;
    bool markedVertex[V];
    bool toVisitVertex[V];
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;

    if (parent_array != NULL)
    {
        (*parent_array) = (int *)calloc(V, sizeof(int));
        parent = (*parent_array);
    }
    else
    {
        parent = calloc(V, sizeof(int));
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
    for (unsigned int i = 0; i < graph[origin]->nb_edges_out; i++)
    {
        dest_vertex_id = graph[origin]->out_edges[i]->succ->id;

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
        for (unsigned int edge_i = 0; edge_i < graph[u]->nb_edges_out; edge_i++)
        {
            dest_vertex_id = graph[u]->out_edges[edge_i]->succ->id;
            if (!vertex_is_in_visibilite(path, dest_vertex_id))
            {
                continue;
            }
            if (!markedVertex[dest_vertex_id] && graph[u]->out_edges[edge_i] != NULL && ((*dist_array)[u] + cost_function(&path->profil, &graph[u]->out_edges[edge_i]->dist, &graph[u]->out_edges[edge_i]->danger) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(&path->profil, &graph[u]->out_edges[edge_i]->dist, &graph[u]->out_edges[edge_i]->danger);
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

double djikstra_backward(struct vertex_t **graph, int V, double **dist_array, int **parent_array, path_t *path)
{
    bool markedVertex[V];
    bool toVisitVertex[V];
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;
    int *parent;

    if (parent_array != NULL)
    {
        (*parent_array) = (int *)calloc(V, sizeof(int));
        parent = (*parent_array);
    }
    else
    {
        parent = (int *)calloc(V, sizeof(int));
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
    for (unsigned int i = 0; i < graph[origin]->nb_edges_in; i++)
    {
        dest_vertex_id = graph[origin]->in_edges[i]->pred->id;

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
        for (unsigned int edge_i = 0; edge_i < graph[u]->nb_edges_in; edge_i++)
        {
            dest_vertex_id = graph[u]->in_edges[edge_i]->pred->id;
            if (!vertex_is_in_visibilite(path, dest_vertex_id))
            {
                continue;
            }
            if (!markedVertex[dest_vertex_id] && graph[u]->in_edges[edge_i] != NULL && ((*dist_array)[u] + cost_function(&path->profil, &graph[u]->in_edges[edge_i]->dist, &graph[u]->in_edges[edge_i]->danger) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(&path->profil, &graph[u]->in_edges[edge_i]->dist, &graph[u]->in_edges[edge_i]->danger);
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