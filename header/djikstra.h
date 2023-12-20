#ifndef DJIKSTRA__H
#define DJIKSTRA__H
#include "edge.h"
#include "path.h"


typedef struct List_node_t
{
    int vertex_id;
    struct List_node_t *next;
} list_node_t;

void get_edges_to_optimize_for_budget(long double budget);
int min_distance(double dist[], list_node_t *vertexToVisit);
void init_cost_diff_array(long double *diff_array, unsigned int nbedge_t);
void get_max_edge_to_optimize(long double *diff_array, unsigned int nb_edges,edge_t **edge_array, int *edge_id_to_optimize, long double budget_left);
double djikstra_test(struct vertex_t *graph, int V, double **dist_array, int **parent, path_t *path);
double djikstra_forward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, path_t *path);
double djikstra_backward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, path_t *path);
double updated_dist(edge_t *edge, path_t *path);
double cost_function(path_t *path, edge_t *edge);

#endif // DJIKSTRA__H