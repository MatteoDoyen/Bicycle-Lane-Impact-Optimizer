#ifndef DJIKSTRA__H
#define DJIKSTRA__H
#include <pthread.h>
#include "edge.h"
#include "path.h"

typedef struct thread_arg_t
{
    path_t ** paths;
    vertex_t** graph;
    uint32_t nb_vertices;
    uint32_t nb_edges;
    uint32_t nb_paths;
    edge_t ** edge_array;
    uint32_t thread_id;
    uint32_t offset;
    long double * cost_diff_array;
    bool * impact;
    pthread_mutex_t *mutex;
    long double *budget_left;
}thread_arg_t;

typedef struct selected_edge_t{
    unsigned int edge_id;
    long double cost_saved;
    struct selected_edge_t *next;
} selected_edge_t;


typedef struct List_node_t
{
    int vertex_id;
    struct List_node_t *next;
} list_node_t;

void print_selected_edges(selected_edge_t * head);
void free_select_edges(selected_edge_t * head);
void new_selected_edge(unsigned edge_id, long double cost_saved, selected_edge_t **head);
double dijistkra_backward_2(int V,double *forward_djikstra, double **backward_djikstra, int *parent_f, double djikstra_cost,path_t * path);
void dijistkra_test(double * forward_djikstra, double ** backward_djikstra,int * parent_f,int ** parent_b,int destination,double djikstra_cost, int V);
void get_edges_to_optimize_for_budget(long double budget, char * graphe_file_name, char * paths_file_name,selected_edge_t ** selected_edges);
void get_edges_to_optimize_for_budget_threaded(long double budget, char * graphe_file_name, char * paths_file_name,int nb_thread,selected_edge_t ** selected_edges);
int min_distance(double dist[], list_node_t *vertexToVisit);
void init_cost_diff_array(long double *diff_array, unsigned int nbedge_t);
void get_max_edge_to_optimize(long double *diff_array,uint32_t nb_edges, edge_t **edge_array, int *edge_id_to_optimize,long double * saved_cost,long double budget_left);
double djikstra_forward(struct vertex_t **graph, int V, double **dist_array, int **parent_array, path_t *path);
double djikstra_backward(struct vertex_t **graph, int V, double **dist_array, int **parent_array, path_t *path);
double updated_dist(edge_t *edge, path_t *path,double *foward_djikstra,double *backward_djikstra);
double cost_function(double *alpha,double * distance,double * danger);

#endif // DJIKSTRA__H