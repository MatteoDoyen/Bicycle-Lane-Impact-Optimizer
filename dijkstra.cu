#include <stdio.h>
#include <stdlib.h>
#include <float.h>
extern "C"{
#include "./header/edge.h"
#include "./header/path.h"
#include "./header/dijkstra.h"
#include "./header/config.h"
}

#define NUM_BLOCK  512  // Number of thread blocks
#define NUM_THREAD  512  // Number of threads per block


__global__ void dijkstra(vertex_cuda_t *graph, double *cost, int *parent, int *visited, int source, uint32_t num_vertices) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (tid < num_vertices) {
        cost[tid] = (tid == source) ? 0 : DBL_MAX;
        parent[tid] = -1;
        visited[tid] = 0;
    }
    
    __syncthreads();
    
    for (int count = 0; count < num_vertices - 1; ++count) {
        double min = DBL_MAX;
        int u = -1;
        
        for (int v = 0; v < num_vertices; ++v) {
            if (!visited[v] && cost[v] < min) {
                min = cost[v];
                u = v;
            }
        }
        
        if (u == -1) {
            break; // No reachable vertices left
        }
        
        visited[u] = 1;
        
        for (int v = 0; v < num_vertices; ++v) {
            double alt = cost[u] + graph[u * num_vertices + v].distance;
            if (!visited[v] && graph[u * num_vertices + v].distance && alt < cost[v]) {
                cost[v] = alt;
                parent[v] = u;
            }
        }
        
        __syncthreads();
    }
}

int main(int argc, char const *argv[]) {
    vertex_cuda_t *graph;
    cifre_conf_t conf;
    set_config(argv[1], &conf);
    uint32_t num_vertices;
    get_graph_cuda(&conf,&graph,&num_vertices);
    // printf("ni %d nj %d dist %f\n",1,1,graph[(1 * (num_vertices)) +1].distance);
    // printf("salut, %d\n",num_vertices);
    // for (int i = 1; i < num_vertices; i++)
    // {
    //     for (int j = 1; j < num_vertices; j++)
    //     {
    //         if(i!=j){
    //             printf("%d %d |  %2f\n",i,j,graph[i*num_vertices + j].distance);
    //         }
    //     }
        
    // }
    
    // Example graph represented as an adjacency matrix
    // int graph[num_vertices][num_vertices] = {
    //     {0, 2, 0, 6, 0},
    //     {0, 0, 3, 8, 5},
    //     {0, 0, 0, 0, 7},
    //     {0, 0, 0, 0, 9},
    //     {0, 0, 0, 0, 0}
    // };

    vertex_cuda_t *d_graph;
    double *d_cost;
    int *d_parent, *d_visited;
    double *cost = (double *) calloc(num_vertices,sizeof(double));
    int *parent = (int * )calloc(num_vertices,sizeof(int));
    int* visited = (int * )calloc(num_vertices,sizeof(int));

    // Allocate memory on GPU
    cudaMalloc((void**)&d_graph, num_vertices * num_vertices * sizeof(vertex_cuda_t));
    cudaMalloc((void**)&d_cost, num_vertices * sizeof(double));
    cudaMalloc((void**)&d_parent, num_vertices * sizeof(int));
    cudaMalloc((void**)&d_visited, num_vertices * sizeof(int));

    // Copy graph to GPU
    cudaMemcpy(d_graph, graph, num_vertices * num_vertices * sizeof(vertex_cuda_t), cudaMemcpyHostToDevice);

    // Choose source vertex
    int source = 1;

    // Run Dijkstra's algorithm on GPU
    dijkstra<<<1, num_vertices>>>(d_graph, d_cost, d_parent, d_visited, source,num_vertices);

    // Copy results back to CPU
    cudaMemcpy(cost, d_cost, num_vertices * sizeof(double), cudaMemcpyDeviceToHost);
    cudaMemcpy(parent, d_parent, num_vertices * sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(visited, d_visited, num_vertices * sizeof(int), cudaMemcpyDeviceToHost);

    // Print the results
    double value;
    printf("Vertex\tCost\tParent\n");
    // for (int i = 0; i < num_vertices; ++i) {
    //     if( cost[i] != DBL_MAX){
    //         printf("%d\t%f\t%d\n", i, cost[i], parent[i]);
    //     }
        
    // }
    int current = 4;
    int old_current = parent[current];
    while(current!=-1){
        printf("%d -> %d : %02f\n",current,old_current,graph[current*num_vertices+old_current].distance);
        old_current = current;
        current = parent[current];
    }
    vertex_t **graph_dij;
    edge_t **edge_array;
    uint32_t nb_vertices, nb_edges, nb_paths;
    get_graph(&conf, &graph_dij, &edge_array, &nb_vertices, &nb_edges);
    double *dist_array_ref = (double *) calloc(nb_vertices,sizeof(double));
    path_t path;
    path.destination = 4;
    path.origin = 1;
    path.profil = 1;

    double dij_cost = dijkstra_backward_full_graph(graph_dij, nb_vertices, &dist_array_ref,NULL, &path);
    printf("%f %f",dij_cost,cost[4]);
    // Free GPU memory
    cudaFree(d_graph);
    cudaFree(d_cost);
    cudaFree(d_parent);
    cudaFree(d_visited);
    free_config(&conf);
    return 0;
}
