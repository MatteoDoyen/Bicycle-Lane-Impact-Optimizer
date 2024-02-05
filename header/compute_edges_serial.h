#ifndef COMPUTE_EDGES_H
#define COMPUTE_EDGES_H
#include "../header/config.h"
#include "edge.h"
#include "path.h"
#include "util.h"
#include "dijkstra.h"


int get_edges_to_optimize_for_budget(cifre_conf_t * config, long double *budget, double_unsigned_list_t **selected_edges);
void save_selected_edges(double_unsigned_list_t *head,char * file_path);
int init_cost_diff_array(double_unsigned_list_t ***diff_array, unsigned int nb_paths);
void get_max_edge_to_optimize(double_unsigned_list_t **diff_array,uint32_t nb_paths, uint32_t nb_edges, edge_t **edge_array, int *edge_id_to_optimize, long double *saved_cost, long double budget_left);
void free_cost_diff_array(double_unsigned_list_t **array, uint32_t nb_paths);

#endif // COMPUTE_EDGES_H