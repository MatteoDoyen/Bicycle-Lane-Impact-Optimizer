#ifndef DJIKSTRA__H
#define DJIKSTRA__H
#include "edge.h"
#include "path.h"

typedef struct cost_diff_edge_t
{
    double djikstra_cost_diff;
    double dist;
} cost_diff_edge_t;

typedef struct List_node_t
{
    int vertex_id;
    struct List_node_t *next;
} list_node_t;

int min_distance(double dist[], list_node_t *vertexToVisit);
void init_cost_diff_array(cost_diff_edge_t *diff_array, unsigned int nbedge_t);
void get_max_edge_to_optimize(cost_diff_edge_t *diff_array, unsigned int nbedge_t, int *edge_id_to_optimize, long double *budget_left);
double djikstra_test(struct vertex_t *graph, int V, double **dist_array, int **parent, path_t *path);
double djikstra_forward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, path_t *path);
double djikstra_backward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, path_t *path);
double updated_dist(edge_t *edge, path_t *path);
double cost_function(path_t *path, edge_t *edge);

#endif // DJIKSTRA__H