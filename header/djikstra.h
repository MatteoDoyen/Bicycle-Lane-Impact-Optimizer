#ifndef DJIKSTRA__H
#define DJIKSTRA__H
#include "arc.h"
#include "trace.h"

typedef struct cost_diff_arc_t{
    unsigned int id_trace;
    double djikstra_cost_diff;
    struct cost_diff_arc_t * suivant;
}cost_diff_arc_t;


int minDistance(double dist[], bool markedVertex[], int V);
void print_cost_diff(cost_diff_arc_t * diff_array, unsigned int nbArc);
void init_cost_diff_array(cost_diff_arc_t * diff_array,unsigned int nbArc);
void get_max_arc_to_optimize(cost_diff_arc_t *diff_array, unsigned int nbArc, unsigned int * arc_id_to_optimize, long double *budget_left);
void new_cost_diff( unsigned int id_trace,double djikstra_cost_diff,cost_diff_arc_t * diff_array);
double djikstra(struct Noeud *graph,int V, double ** dist_array, trace_t *trace);
double updated_dist(Arc *arc, trace_t * trace);
double cost_function( trace_t * trace, Arc *arc);

#endif // DJIKSTRA__H