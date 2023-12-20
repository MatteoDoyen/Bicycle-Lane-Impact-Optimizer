#ifndef DJIKSTRA__H
#define DJIKSTRA__H
#include "arc.h"
#include "trace.h"

typedef struct cost_diff_arc_t
{
    double djikstra_cost_diff;
    double dist;
} cost_diff_arc_t;

typedef struct List_node_t
{
    int vertex_id;
    struct List_node_t *next;
} list_node_t;

int minDistance(double dist[], list_node_t *vertexToVisit);
void init_cost_diff_array(cost_diff_arc_t *diff_array, unsigned int nbArc);
void get_max_arc_to_optimize(cost_diff_arc_t *diff_array, unsigned int nbArc, int *arc_id_to_optimize, long double *budget_left);
double djikstra_test(struct Noeud *graph, int V, double **dist_array, int **parent, trace_t *trace);
double djikstra_forward(struct Noeud *graph, int V, double **dist_array, int **parent_array, trace_t *trace);
double djikstra_backward(struct Noeud *graph, int V, double **dist_array, int **parent_array, trace_t *trace);
double updated_dist(Arc *arc, trace_t *trace);
double cost_function(trace_t *trace, Arc *arc);

#endif // DJIKSTRA__H