#ifndef PATH_H
#define PATH_H

#include "../header/edge.h"
#include "../header/config.h"

#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>

// dummy data
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

// real data
// #define T_ORIGIN_INDEX 1
// #define T_DESTINATION_INDEX 2
// #define T_CHEMIN_INDEX 3
// #define T_DISTANCE_INDEX 4
// #define T_DANGER_INDEX 5
// #define T_VISIBILITE_INDEX 9
// #define T_CPC_INDEX 10
// #define T_DIST_CPS_INDEX 11
// #define T_DANGER_CPS_INDEX 12
// #define T_PROFIL_INDEX 13

typedef struct
{
    uint32_t origin;
    uint32_t destination;
    double profil;
    double distance;
    double danger;
    uint32_t *chemin;
    uint32_t *djikstra_sp;
    uint32_t nb_djikstra_sp;
    uint32_t nb_chemin;
    uint32_t *visibilite;
    uint32_t nb_visibilite;
    double djikstra_dist;
    double cps_djikstra_dist;
    double cps_djikstra_danger;
    double *foward_djikstra;
    double *backward_djikstra;
} path_t;

int get_paths(cifre_conf_t *conf, path_t ***paths, uint32_t *nb_paths);
bool edge_is_in_visibilite(path_t *paths, edge_t *edge);
bool troncon_is_in_path(path_t *paths, edge_t *edge);
bool vertex_is_in_visibilite(path_t *paths, uint32_t vertex_id);
void free_paths(path_t** paths,uint32_t nb_paths);

#endif // PATH_H