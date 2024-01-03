#ifndef DJIKSTRA__H
#define DJIKSTRA__H
#include <pthread.h>
#include "edge.h"
#include "path.h"

typedef struct List_node_t
{
    int vertex_id;
    struct List_node_t *next;
} list_node_t;

int min_distance(double dist[], list_node_t *vertexToVisit);
double djikstra_forward(struct vertex_t **graph, int V, double **dist_array, int **parent_array, path_t *path);
double djikstra_backward(struct vertex_t **graph, int V, double **dist_array, int **parent_array, path_t *path);
double updated_dist(edge_t *edge, path_t *path,double *foward_djikstra,double *backward_djikstra);
double cost_function(double *alpha,double * distance,double * danger);

#endif // DJIKSTRA__H