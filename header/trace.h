#ifndef TRACE_H
#define TRACE_H

#include "../header/arc.h"
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>

#define T_ORIGIN_INDEX 1
#define T_DESTINATION_INDEX 2
#define T_PROFIL_INDEX 3
#define T_DISTANCE_INDEX 4
#define T_DANGER_INDEX 5
#define T_CHEMIN_INDEX 6
#define T_CPC_INDEX 10
#define T_DIST_CPS_INDEX 11
#define T_DANGER_CPS_INDEX 12
#define T_VISIBILITE_INDEX 13

typedef struct
{
    unsigned int origin;
    unsigned int destination;
    double profil;
    double distance;
    double danger;
    unsigned int *chemin;
    unsigned int *djikstra_sp;
    unsigned int nb_djikstra_sp;
    unsigned int nb_chemin;
    unsigned int *visibilite;
    unsigned int nb_visibilite;
    double djikstra_dist;
    double cps_djikstra_dist;
    double cps_djikstra_danger;
    double *foward_djikstra;
    double *backward_djikstra;
} trace_t;

trace_t *get_traces(char *csv_path, char *csv_delimiter, int *nb_traces);
bool arcIsInVisiblite(trace_t *traces, Arc *arc);
bool tronconIsInTrace(trace_t *traces, Arc *arc);
bool vertexIsInVisiblite(trace_t *traces, unsigned int vertex_id);
void free_traces(trace_t* traces,int nb_traces);

#endif