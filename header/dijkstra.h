#ifndef dijkstra__H
#define dijkstra__H
#include "graph.h"
#include "util.h"
#include "path.h"


int min_distance(double dist[], unsigned_list_t *vertexToVisit);
double dijkstra_bidirectional(graph_t *graph, double **dist_array_ref, int *parents, path_t *path, bool is_forward);
double updated_dist(edge_t *edge,double *foward_dijkstra,double *backward_dijkstra);
double cost_function(double alpha,double * distance,double * danger);

#endif // dijkstra__H