#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "../header/edge.h"
#include "../header/path.h"
#include "../header/dijkstra.h"
#include "../header/display_progress.h"
#include "../header/config.h"
#include "../header/util.h"
#include "../header/dijkstra_cuda.cuh"
}

extern "C"
int main(int argc, char const *argv[])
{
    vertex_t **graph;
    path_t **paths;
    edge_t **edge_array;
    int ret_code;
    uint32_t nb_vertices, nb_edges, nb_paths;
    cifre_conf_t config;
    int *parents;
    double *dijkstra_forward_dist;
    
    set_config(argv[1], &config);
    dijkstra_forward_dist =(double *) calloc(nb_vertices, sizeof(double));
    ret_code = get_graph(&config, &graph, &edge_array, &nb_vertices, &nb_edges);
    ret_code = get_paths(&config, &paths, &nb_paths);

    parents = (int *)calloc(nb_vertices,sizeof(int));
    dijkstra_forward_cuda(graph, nb_vertices, &dijkstra_forward_dist, NULL, paths[0]);
    return 0;
}
