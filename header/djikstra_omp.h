#ifndef DJIKSTRA_OMP__H
#define DJIKSTRA_OMP__H

#include "edge.h"
#include "util.h"
#include "path.h"

int min_distance_full_graph_omp(double dist[], bool markedVertex[], int V);
double djikstra_backward_full_graph_omp(struct vertex_t **graph, int nb_vertices, double **dist_array_ref, int *parents, path_t *path);
double djikstra_backward_small_graph_omp(struct vertex_t **graph, int nb_vertices, double **dist_array_ref, int *parents, path_t *path);

#endif // DJIKSTRA_OMP__H