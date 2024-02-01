#ifndef COMPUTE_EDGES_PTHREAD_H
#define COMPUTE_EDGES_PTHREAD_H
#include <pthread.h>
#include "../header/config.h"
#include "../header/compute_edges_serial.h"

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

void *compute_optimize_for_budget_threaded(void *arg);
int get_edges_to_optimize_for_budget_pthread(cifre_conf_t * config,long double *budget_used, double_unsigned_list_t **selected_edges);

#endif // COMPUTE_EDGES_PTHREAD_H