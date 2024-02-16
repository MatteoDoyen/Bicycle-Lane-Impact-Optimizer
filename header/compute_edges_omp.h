#ifndef COMPUTE_EDGES_OMP_H
#define COMPUTE_EDGES_OMP_H

#include "../header/config.h"
#include "../header/compute_edges_serial.h"

int get_edges_to_optimize_for_budget_omp(config_t * config,long double *budget_used, double_unsigned_list_t **selected_edges);

#endif // COMPUTE_EDGES_OMP_H