#ifndef DJIKSTRA__CUDA__H
#define DJIKSTRA__CUDA__H
#include "edge.h"
#include "util.h"
#include "path.h"

#define THREADS_BLOCK 512

#define CUDA_SAFE_CALL(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line, bool abort = true)
{
    if (code != cudaSuccess)
    {
        fprintf(stderr, "CUDA_SAFE_CALL: %s %s %d\n", cudaGetErrorString(code), file, line);
        if (abort) exit(code);
    }
}

int min_distance_cuda(double dist[], unsigned_list_t *vertexToVisit);
double djikstra_forward_cuda(struct vertex_t **graph, int V, double **dist_array, int *parent_array, path_t *path);
__global__ void cudaRelax(vertex_t **graph, double *dist_array, int *parents, bool *marked_vertex, int *global_min_index, path_t *path);
__global__ void min_distance(double *dist, bool *marked_vertex, int *global_min_index, int nb_vertices);

#endif // DJIKSTRA__H