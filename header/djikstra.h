#ifndef DJIKSTRA__H
#define DJIKSTRA__H
#include <pthread.h>
#include "edge.h"
#include "path.h"

typedef struct thread_arg_t
{
    path_t * paths;
    vertex_t* graph;
    int nb_vertices;
    int nb_edges;
    int nb_paths;
    edge_t ** edge_array;
    unsigned int thread_id;
    unsigned int offset;
    long double * cost_diff_array;
    pthread_mutex_t *mutex;
}thread_arg_t;


typedef struct List_node_t
{
    int vertex_id;
    struct List_node_t *next;
} list_node_t;

void get_edges_to_optimize_for_budget(long double budget, char * graphe_file_name, char * paths_file_name);
void get_edges_to_optimize_for_budget_threaded(long double budget, char * graphe_file_name, char * paths_file_name,int nb_thread);
int min_distance(double dist[], list_node_t *vertexToVisit);
void init_cost_diff_array(long double *diff_array, unsigned int nbedge_t);
void get_max_edge_to_optimize(long double *diff_array, unsigned int nb_edges,edge_t **edge_array, int *edge_id_to_optimize, long double budget_left);
double djikstra_forward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, path_t *path);
double djikstra_backward(struct vertex_t *graph, int V, double **dist_array, int **parent_array, path_t *path);
double updated_dist(edge_t *edge, path_t *path,double *foward_djikstra,double *backward_djikstra);
double cost_function(double *alpha,double * distance,double * danger);

#endif // DJIKSTRA__H