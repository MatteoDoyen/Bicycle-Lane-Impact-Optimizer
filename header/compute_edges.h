#ifndef COMPUTE_EDGES_H
#define COMPUTE_EDGES_H
#include "../header/config.h"
#include <pthread.h>
#include "edge.h"
#include "path.h"
#include "util.h"
#include "djikstra.h"

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
    double_unsigned_list_t ** cost_diff_array;
    bool * impact;
    pthread_mutex_t *mutex;
    long double *budget_left;
}thread_arg_t;


int get_edges_to_optimize_for_budget(cifre_conf_t * config, long double *budget, double_unsigned_list_t **selected_edges);
int get_edges_to_optimize_for_budget_threaded(cifre_conf_t * config,long double *budget_used, double_unsigned_list_t **selected_edges);

void save_selected_edges(double_unsigned_list_t *head,char * file_path);
int init_cost_diff_array(double_unsigned_list_t ***diff_array, unsigned int nb_paths);
void get_max_edge_to_optimize(double_unsigned_list_t **diff_array,uint32_t nb_paths, uint32_t nb_edges, edge_t **edge_array, int *edge_id_to_optimize, long double *saved_cost, long double budget_left);
void free_improved_edge_array(double_unsigned_list_t **array, uint32_t nb_paths);

#endif // COMPUTE_EDGES_H