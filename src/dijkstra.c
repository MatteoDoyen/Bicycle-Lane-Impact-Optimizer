#include "../header/dijkstra.h"
#include <stdbool.h>
#include <stdlib.h>
#include <omp.h>


/*
 * Function: cost_function
 * ----------------------------------
 * This function weight of an edge based on the given alpha of a path, and the distance and danger of an edge.
 *
 * Parameters:
 * - alpha: a pointer to the alpha value
 * - distance: a pointer to the distance value
 * - danger: a pointer to the danger value
 *
 * Returns:
 * - The precomputed cost
 *
 * Note:
 * - Make sure to dereference the pointers before performing the calculations.
 * - The cost is precomputed using the formula: (alpha * distance) + ((1 - alpha) * danger)
 *
 * Example usage:
 * - double alpha = 0.5;
 *   double distance = 10.0;
 *   double danger = 0.8;
 *   double cost = cost_function(alpha, &distance, &danger);
 */
double cost_function(double alpha, double *distance, double *danger)
{
    return ((alpha) * (*distance)) + ((1 - (alpha)) * (*danger));
}

/**
 * Function: updated_dist
 * ----------------------------------
 * This function calculates the updated distance in the cached distance precomputed by Dijkstra.
 *
 * Parameters:
 * - edge: Pointer to the edge structure.
 * - forward_dijkstra: Pointer to the array storing the forward Dijkstra distances.
 * - backward_dijkstra: Pointer to the array storing the backward Dijkstra distances.
 *
 * Returns:
 * - The updated distance.
 *
 * Note:
 * - Make sure to update the distance of the vertex before calling this function.
 *
 * Example usage:
 * - double distance = updated_dist(&edge, forward_dijkstra, backward_dijkstra);
 */
double updated_dist(edge_t *edge, double *forward_dijkstra, double *backward_dijkstra)
{
    // An optimized edge's weight is just its distance
    return forward_dijkstra[edge->pred->id] + edge->dist + backward_dijkstra[edge->succ->id];
}

/*
 * Function: dijkstra_bidirectional
 * ----------------------------------
 * This function calculates the shortest path from a given origin vertex to a destination vertex in a graph using Dijkstra's algorithm.
 * If is_forward equal false then it find the shortest path from the destination vertex to the origin vertex.
 * The dist_array gets updated with the shortest distance from the origin or destination vertex (if is_forward is false) to all other vertices.
 * Parameters:
 * - graph: Pointer to the graph structure.
 * - dist_array_ref: Pointer to the distance array reference.
 * - parents: Pointer to the array of parent vertices.
 * - path: Pointer to the path structure.
 * - is_forward: Boolean, indicate if the dijkstra is forward or backward
 *
 * Returns:
 * - The shortest distance from the origin vertex to the destination vertex.
 *
 * Note:
 * - The function assumes that the graph, dist_array_ref, and path are valid pointers.
 * - The function modifies the dist_array, marked_vertex, visitable_vertex, and parents arrays.
 * - The function uses the cost_function and vertex_is_in_visibilite functions.
 * - The function frees the vertices_to_visit list before returning.
 *
 * Example usage:
 * - double distance = dijkstra_bidirectional(&graph, &dist_array, parents, &path, true);
 */
double dijkstra_bidirectional(graph_t *graph, double **dist_array_ref, int *parents, path_t *path, bool is_forward)
{
    bool marked_vertex[graph->nb_vertices];
    bool visitable_vertex[graph->nb_vertices];
    int dest_vertex_id;
    int min_dist_index;
    unsigned int nb_edges;
    double *dist_array = *dist_array_ref;
    unsigned int origin = is_forward ? path->origin : path->destination;
    unsigned int destination = is_forward ? path->destination : path->origin;
    unsigned_list_t *vertices_to_visit = NULL;
    edge_t *current_edge;
    double current_cost;
    vertex_t *current_vertex;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (uint32_t i = 0; i < graph->nb_vertices; i++)
    {
        dist_array[i] = DBL_MAX;
        marked_vertex[i] = false;
        visitable_vertex[i] = false;
    }

    // Distance from source to itself is always 0
    dist_array[origin] = 0;
    if (parents != NULL)
    {
        parents[origin] = -1;
    }

    add_unsigned_list_t(&vertices_to_visit, origin);
    nb_edges = is_forward ? graph->vertex_array[origin]->nb_edges_out : graph->vertex_array[origin]->nb_edges_in;
    for (unsigned int i = 0; i < nb_edges; i++)
    {
        dest_vertex_id = is_forward ? graph->vertex_array[origin]->out_edges[i]->succ->id : graph->vertex_array[origin]->in_edges[i]->pred->id;
        if (vertex_is_in_visibilite(path, dest_vertex_id))
        {
            visitable_vertex[dest_vertex_id] = true;
            add_unsigned_list_t(&vertices_to_visit, dest_vertex_id);
        }
    }

    // Find the shortest path for all vertices in the visibility
    while (vertices_to_visit != NULL)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        min_dist_index = min_distance(dist_array, vertices_to_visit);
        if (min_dist_index == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        marked_vertex[min_dist_index] = true;
        // Remove the picked vertex from the list of vertices to visit
        delete_value_in_unsigned_list(&vertices_to_visit, min_dist_index);

        current_vertex = graph->vertex_array[min_dist_index];

        nb_edges = is_forward ? current_vertex->nb_edges_out : current_vertex->nb_edges_in;
        // Update dist value of the adjacent vertices
        for (unsigned int edge_i = 0; edge_i < nb_edges; edge_i++)
        {
            current_edge = is_forward ? current_vertex->out_edges[edge_i] : current_vertex->in_edges[edge_i];
            dest_vertex_id = is_forward ? current_edge->succ->id : current_edge->pred->id;
            current_cost = cost_function(path->profil, &current_edge->dist, &current_edge->danger);

            // Ensure that the vertex is in the visibility, not marked and the new distance is less than the current distance
            if (vertex_is_in_visibilite(path, dest_vertex_id) && !marked_vertex[dest_vertex_id] && ((dist_array[min_dist_index] + current_cost) < dist_array[dest_vertex_id]))
            {
                dist_array[dest_vertex_id] = dist_array[min_dist_index] + current_cost;
                if (parents != NULL)
                {
                    parents[dest_vertex_id] = min_dist_index;
                }
                if (!visitable_vertex[dest_vertex_id])
                {
                    // Mark the picked vertex as a candidat to process
                    visitable_vertex[dest_vertex_id] = true;
                    add_unsigned_list_t(&vertices_to_visit, dest_vertex_id);
                }
            }
        }
    }
    free_unsigned_list_t(vertices_to_visit);
    return dist_array[destination];
}

/*
 * Function: min_distance
 * ----------------------------------
 * This function returns the index of the vertex with the minimum distance from the origin vertex.
 *
 * Parameters:
 * - dist: The array of distances.
 * - vertexToVisit: Pointer to the list of vertices to visit.
 *
 * Returns:
 * - The index of the vertex with the minimum distance from the origin vertex.
 *
 * Example usage:
 * - int min_index = min_distance(dist, vertexToVisit);
 */
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