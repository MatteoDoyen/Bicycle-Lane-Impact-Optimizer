#ifndef dijkstra_OMP__H
#define dijkstra_OMP__H

#include "edge.h"
#include "util.h"
#include "path.h"

int min_distance_full_graph_omp(double dist[], bool markedVertex[], int V);
double dijkstra_backward_full_graph_omp(struct vertex_t **graph, int nb_vertices, double **dist_array_ref, int *parents, path_t *path);
double dijkstra_backward_small_graph_omp(struct vertex_t **graph, int nb_vertices, double **dist_array_ref, int *parents, path_t *path);

#endif // dijkstra_OMP__H