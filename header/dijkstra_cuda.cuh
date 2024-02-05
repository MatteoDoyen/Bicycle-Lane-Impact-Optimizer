#ifndef dijkstra__CUDA__H
#define dijkstra__CUDA__H
extern "C"{
    #include "edge.h"
    #include "util.h"
    #include "path.h"
}


#define THREADS_BLOCK 512

extern "C"
void allocate_path_on_gpu(path_t **d_path, path_t *h_path);
extern "C"
void allocate_graph_on_gpu(vertex_t **graph_gpu, vertex_t **graph_cpu, uint32_t num_vertices);
extern "C"
double dijkstra_forward_cuda(struct vertex_t **graph, int V, double **dist_array, int *parent_array, path_t *path);
extern "C"
__global__ void cudaRelax(vertex_t **graph, double *dist_array, int *parents, bool *marked_vertex, int *global_min_index, path_t *path);
extern "C"
__global__ void min_distance_cuda(double *dist, bool *marked_vertex, int *global_min_index, int *nb_vertices);
#endif // dijkstra__H