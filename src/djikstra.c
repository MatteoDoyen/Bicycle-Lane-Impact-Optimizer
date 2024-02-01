#include "../header/djikstra.h"
#include <stdbool.h>
#include <stdlib.h>
#include <omp.h>

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

double djikstra_forward(struct vertex_t **graph, int nb_vertices, double **dist_array_ref, int *parents, path_t *path)
{
    bool markedVertex[nb_vertices];
    bool toVisitVertex[nb_vertices];
    int dest_vertex_id;
    int min_dist_index;
    double *dist_array = *dist_array_ref;
    unsigned int origin = path->origin;
    unsigned int destination = path->destination;
    unsigned_list_t *verticesToVisit = NULL;
    edge_t *current_edge;
    double current_cost;
    vertex_t *current_vertex;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < nb_vertices; i++)
    {
        dist_array[i] = DBL_MAX;
        markedVertex[i] = false;
        toVisitVertex[i] = false;
    }

    // Distance from source to itself is always 0
    dist_array[origin] = 0;
    if (parents != NULL)
    {
        parents[origin] = -1;
    }

    add_unsigned_list_t(&verticesToVisit, origin);
    for (unsigned int i = 0; i < graph[origin]->nb_edges_out; i++)
    {
        dest_vertex_id = graph[origin]->out_edges[i]->succ->id;
        toVisitVertex[dest_vertex_id] = true;
        add_unsigned_list_t(&verticesToVisit, dest_vertex_id);
    }

    // Find the shortest path for all vertices
    while (verticesToVisit != NULL)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        min_dist_index = min_distance(dist_array, verticesToVisit);
        if (min_dist_index == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[min_dist_index] = true;

        delete_value_in_unsigned_list(&verticesToVisit, min_dist_index);

        current_vertex = graph[min_dist_index];
        // Update dist value of the adjacent vertices
        for (unsigned int edge_i = 0; edge_i < current_vertex->nb_edges_out; edge_i++)
        {
            current_edge = current_vertex->out_edges[edge_i];
            dest_vertex_id = current_edge->succ->id;
            current_cost = cost_function(&path->profil, &current_edge->dist, &current_edge->danger);

            dest_vertex_id = current_edge->succ->id;
            if (vertex_is_in_visibilite(path, dest_vertex_id) && !markedVertex[dest_vertex_id] && ((dist_array[min_dist_index] + current_cost) < dist_array[dest_vertex_id]))
            {
                dist_array[dest_vertex_id] = dist_array[min_dist_index] + current_cost;
                if (parents != NULL)
                {
                    parents[dest_vertex_id] = min_dist_index;
                }
                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    add_unsigned_list_t(&verticesToVisit, dest_vertex_id);
                }
            }
        }
    }
    free_unsigned_list_t(verticesToVisit);
    return dist_array[destination];
}

double djikstra_backward(struct vertex_t **graph, int nb_vertices, double **dist_array_ref, int *parents, path_t *path)
{
    bool markedVertex[nb_vertices];
    bool toVisitVertex[nb_vertices];
    int dest_vertex_id;
    double *dist_array = *dist_array_ref;
    int min_dist_index;
    unsigned int origin = path->destination;
    unsigned int destination = path->origin;
    unsigned_list_t *verticesToVisit = NULL;
    edge_t *current_edge;
    double current_cost;
    vertex_t *current_vertex;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < nb_vertices; i++)
    {
        dist_array[i] = DBL_MAX;
        markedVertex[i] = false;
        toVisitVertex[i] = false;
    }

    // Distance from source to itself is always 0
    dist_array[origin] = 0;
    if (parents != NULL)
    {
        parents[origin] = -1;
    }

    add_unsigned_list_t(&verticesToVisit, origin);

    for (unsigned int i = 0; i < graph[origin]->nb_edges_in; i++)
    {
        dest_vertex_id = graph[origin]->in_edges[i]->pred->id;
        toVisitVertex[dest_vertex_id] = true;
        add_unsigned_list_t(&verticesToVisit, dest_vertex_id);
    }

    // Find the shortest path for all vertices
    while (verticesToVisit != NULL)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        min_dist_index = min_distance(dist_array, verticesToVisit);
        if (min_dist_index == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[min_dist_index] = true;

        delete_value_in_unsigned_list(&verticesToVisit, min_dist_index);

        // Update dist value of the adjacent vertices
        current_vertex = graph[min_dist_index];

        for (unsigned int edge_i = 0; edge_i < current_vertex->nb_edges_in; edge_i++)
        {
            current_edge = current_vertex->in_edges[edge_i];
            dest_vertex_id = current_edge->pred->id;
            current_cost = cost_function(&path->profil, &current_edge->dist, &current_edge->danger);

            if (vertex_is_in_visibilite(path, dest_vertex_id) && !markedVertex[dest_vertex_id] && ((dist_array[min_dist_index] + current_cost) < dist_array[dest_vertex_id]))
            {
                dist_array[dest_vertex_id] = dist_array[min_dist_index] + current_cost;
                if (parents != NULL)
                {
                    parents[dest_vertex_id] = min_dist_index;
                }
                // Mark the picked vertex as a candidat to process
                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    add_unsigned_list_t(&verticesToVisit, dest_vertex_id);
                }
            }
        }
    }

    free_unsigned_list_t(verticesToVisit);

    return dist_array[destination];
}

double djikstra_backward_full_graph(struct vertex_t **graph, int nb_vertices, double **dist_array_ref, int *parents, path_t *path)
{
    bool markedVertex[nb_vertices];
    int dest_vertex_id;
    double *dist_array = *dist_array_ref;
    int min_dist_index;
    unsigned int origin = path->destination;
    unsigned int destination = path->origin;
    edge_t *current_edge;
    double current_cost;
    vertex_t *current_vertex;

 // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < nb_vertices; i++)
    {
        dist_array[i] = DBL_MAX;
        markedVertex[i] = false;
    }

    // Distance from source to itself is always 0
    dist_array[origin] = 0;

    if(parents!=NULL){
    // Parent of source is itself
        parents[origin] = -1;
    }

    // Find the shortest path for all vertices
    for (int count = 0; count < nb_vertices - 1; count++)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        min_dist_index = min_distance_full_graph(dist_array, markedVertex, nb_vertices);
        if (min_dist_index == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[min_dist_index] = true;
        current_vertex = graph[min_dist_index];
        // Update dist value of the adjacent vertices
        for (unsigned int edge_i = 0; edge_i < current_vertex->nb_edges_in; edge_i++)
        {
            current_edge = current_vertex->in_edges[edge_i];
            dest_vertex_id = current_edge->pred->id;
            current_cost = cost_function(&path->profil, &current_edge->dist, &current_edge->danger);

            // if(!vertex_is_in_visibilite(path, dest_vertex_id)){
            //     continue;
            // }
            if (!markedVertex[dest_vertex_id] && ((dist_array[min_dist_index] + current_cost) < dist_array[dest_vertex_id]))
            {
                dist_array[dest_vertex_id] = dist_array[min_dist_index] + current_cost;
                if (parents != NULL)
                {
                    parents[dest_vertex_id] = min_dist_index;
                }
            }
        }
    }
    return dist_array[destination];
}

int min_distance(double dist[], unsigned_list_t *vertexToVisit)
{
    double min = DBL_MAX;
    int min_index = -1;

    unsigned_list_t *temp = vertexToVisit;

    while (temp != NULL)
    {
        if (dist[temp->u_value] < min)
        {
            min = dist[temp->u_value];
            min_index = temp->u_value;
        }
        temp = temp->next;
    }
    return min_index;
}

int min_distance_full_graph(double dist[], bool markedVertex[], int V)
{
    double min = DBL_MAX;
    int min_index = -1;

    for (int v = 0; v < V; v++)
    {
        if ((markedVertex[v] == false) && dist[v] < min)
        {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}