#ifndef COMPUTE_EDGES_H
#define COMPUTE_EDGES_H
#include <pthread.h>
#include "edge.h"
#include "path.h"
#include "djikstra.h"



typedef struct selected_edge_t{
    unsigned int edge_id;
    long double cost_saved;
    struct selected_edge_t *next;
} selected_edge_t;

typedef struct selected_edge_t improved_edge_t;

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
    improved_edge_t ** cost_diff_array;
    bool * impact;
    pthread_mutex_t *mutex;
    long double *budget_left;
}thread_arg_t;


void print_selected_edges(selected_edge_t * head);
void free_select_edges(selected_edge_t * head);
int new_selected_edge(unsigned edge_id, long double cost_saved, selected_edge_t **head);
int get_edges_to_optimize_for_budget(long double budget, char * graphe_file_name, char * paths_file_name,selected_edge_t ** selected_edges);
int get_edges_to_optimize_for_budget_threaded(long double budget, char * graphe_file_name, char * paths_file_name,int nb_thread,selected_edge_t ** selected_edges);
void init_cost_diff_array(improved_edge_t **diff_array, unsigned int nbedge_t);
int new_cost_diff(improved_edge_t **diff_path_array, uint32_t edge_id, long double cost_difference);
void get_max_edge_to_optimize(long double *diff_array,uint32_t nb_edges, edge_t **edge_array, int *edge_id_to_optimize,long double * saved_cost,long double budget_left);

#endif // COMPUTE_EDGES_H