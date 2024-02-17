#ifndef PATH_H
#define PATH_H

#include "../header/graph.h"
#include "../header/config.h"

#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <float.h>

typedef struct
{
    uint32_t origin;
    uint32_t destination;
    double profil;
    double distance;
    double danger;
    uint32_t *chemin;
    uint32_t *dijkstra_sp;
    uint32_t nb_dijkstra_sp;
    uint32_t nb_chemin;
    uint32_t *visibilite;
    uint32_t nb_visibilite;
    double dijkstra_dist;
    double cps_dijkstra_dist;
    double cps_dijkstra_danger;
} path_t;

int get_paths(config_t *conf, path_t ***paths, uint32_t *nb_paths);
bool edge_is_in_visibilite(path_t *paths, edge_t *edge);
bool vertex_is_in_visibilite(path_t *paths, uint32_t vertex_id);
void free_paths(path_t** paths,uint32_t nb_paths);

#endif // PATH_H