#ifndef DJIKSTRA__H
#define DJIKSTRA__H
#include "edge.h"
#include "trace.h"

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

int minDistance(double dist[], list_node_t *vertexToVisit);
void init_cost_diff_array(cost_diff_edge_t *diff_array, unsigned int nbedge_t);
void get_max_edge_to_optimize(cost_diff_edge_t *diff_array, unsigned int nbedge_t, int *edge_id_to_optimize, long double *budget_left);
double djikstra_test(struct vertex_t *graph, int V, double **dist_array, int **parent, trace_t *trace);
double djikstra_forward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, trace_t *trace);
double djikstra_backward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, trace_t *trace);
double updated_dist(edge_t *edge, trace_t *trace);
double cost_function(trace_t *trace, edge_t *edge);

#endif // DJIKSTRA__H