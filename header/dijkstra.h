#ifndef dijkstra__H
#define dijkstra__H
#include "graph.h"
#include "util.h"
#include "path.h"


int min_distance_full_graph(double dist[], bool markedVertex[], int V);
double dijkstra_backward_full_graph(struct vertex_t **graph, int nb_vertices, double **dist_array_ref, int *parents, path_t *path);

int min_distance(double dist[], unsigned_list_t *vertexToVisit);
double dijkstra_forward(graph_t *graph, double **dist_array_ref, int *parents, path_t *path);
double dijkstra_backward(graph_t *graph, double **dist_array_ref, int *parents, path_t *path);
double updated_dist(edge_t *edge, path_t *path,double *foward_dijkstra,double *backward_dijkstra);
double cost_function(double *alpha,double * distance,double * danger);

#endif // dijkstra__H