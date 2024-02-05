#include "../header/dijkstra.h"
#include "../header/dijkstra_cuda.cuh"
#include <stdbool.h>
#include <stdlib.h>

void allocate_graph_on_gpu(vertex_t **graph_gpu, vertex_t **graph_cpu, uint32_t num_vertices)
{

    cudaMalloc(graph_gpu, num_vertices * sizeof(vertex_t *));
    cudaMemcpy(graph_gpu, graph_cpu, num_vertices * sizeof(vertex_t *), cudaMemcpyHostToDevice);
    printf("allocate_graph %d\n",num_vertices);
    // Allocate memory for each vertex and edge on the GPU
    for (uint32_t i = 0; i < num_vertices; i++)
    {

        printf("boucle %d\n", graph_cpu[i]->nb_edges_in);
        if (graph_cpu[i]->nb_edges_in > 0)
        {
            cudaMalloc(graph_gpu[i]->in_edges, graph_cpu[i]->nb_edges_in * sizeof(edge_t *));
            cudaMemcpy(graph_gpu[i]->in_edges, graph_cpu[i]->in_edges, graph_cpu[i]->nb_edges_in * sizeof(edge_t *), cudaMemcpyHostToDevice);
        }

        printf("midle\n");
        if (graph_cpu[i]->nb_edges_out > 0)
        {
            cudaMalloc(graph_gpu[i]->out_edges, graph_cpu[i]->nb_edges_out * sizeof(edge_t *));
            cudaMemcpy(graph_gpu[i]->out_edges, graph_cpu[i]->out_edges, graph_cpu[i]->nb_edges_out * sizeof(edge_t *), cudaMemcpyHostToDevice);
        }
    }
}

void free_path_on_gpu(path_t *d_path)
{
    // Free memory on GPU
    cudaFree(d_path->chemin);
    cudaFree(d_path->dijkstra_sp);
    cudaFree(d_path->visibilite);
}

void allocate_path_on_gpu(path_t **d_path, path_t *h_path)
{
    // Copy simple data types
    cudaMemcpy(&((*d_path)->origin), &(h_path->origin), sizeof(uint32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->destination), &(h_path->destination), sizeof(uint32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->profil), &(h_path->profil), sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->distance), &(h_path->distance), sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->danger), &(h_path->danger), sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->nb_dijkstra_sp), &(h_path->nb_dijkstra_sp), sizeof(uint32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->nb_chemin), &(h_path->nb_chemin), sizeof(uint32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->nb_visibilite), &(h_path->nb_visibilite), sizeof(uint32_t), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->dijkstra_dist), &(h_path->dijkstra_dist), sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->cps_dijkstra_dist), &(h_path->cps_dijkstra_dist), sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(&((*d_path)->cps_dijkstra_danger), &(h_path->cps_dijkstra_danger), sizeof(double), cudaMemcpyHostToDevice);

    // Allocate memory for arrays on GPU
    cudaMalloc((void **)&((*d_path)->chemin), h_path->nb_chemin * sizeof(uint32_t));
    cudaMalloc((void **)&((*d_path)->dijkstra_sp), h_path->nb_dijkstra_sp * sizeof(uint32_t));
    cudaMalloc((void **)&((*d_path)->visibilite), h_path->nb_visibilite * sizeof(uint32_t));
}

double dijkstra_forward_cuda(vertex_t **graph, int nb_vertices, double **dist_array_ref, int *parents, path_t *path)
{
    printf("la la\n");
    double *dist_array = *dist_array_ref;
    bool marked_vertex[nb_vertices];
    unsigned int origin = path->destination;
    unsigned int destination = path->origin;

    vertex_t **gpu_graph;
    double *gpu_dist_array;
    int *gpu_parents;
    path_t *gpu_path;
    bool *gpu_marked_vertex;
    int *gpu_nb_vertices;

    int graph_size = (sizeof(vertex_t *) * nb_vertices) + (sizeof(vertex_t) * nb_vertices);
    int dist_array_size = nb_vertices * sizeof(double);
    int marked_vertex_size = nb_vertices * sizeof(bool);
    int parents_size = nb_vertices * sizeof(int);
    int path_size = nb_vertices * sizeof(path_t);

    // Min:  One thread checks for closest vertex. Ideally there would be multiple threads working in
    //  parallel, but due to compiler issues with prallelized-reduction functions this is being used as a backup.
    dim3 gridMin(1, 1, 1);
    dim3 blockMin(1, 1, 1);

    // Relax: Each thread is responsible for relaxing from a shared, given vertex
    //   to one other vertex determined by the ID of the thread. Since each thread handles
    //   a different vertex, there's no RaW or WaR data hazards; all that's needed is a
    //   __syncthreads(); call at the end to ensure all either update or do nothing.
    dim3 gridRelax(nb_vertices / THREADS_BLOCK, 1, 1);
    dim3 blockRelax(THREADS_BLOCK, 1, 1);

    // for closest vertex
    int *closest_vertex = (int *)malloc(sizeof(int));
    int *gpu_closest_vertex;
    closest_vertex[0] = -1;
    cudaMalloc((void **)&gpu_closest_vertex, (sizeof(int)));
    cudaMemcpy(gpu_closest_vertex, closest_vertex, sizeof(int), cudaMemcpyHostToDevice);

    cudaMalloc((void **)&gpu_nb_vertices, (sizeof(int)));
    cudaMemcpy(&gpu_nb_vertices[0], &nb_vertices, sizeof(int), cudaMemcpyHostToDevice);
    printf("2\n");
    allocate_path_on_gpu(&gpu_path, path);
    allocate_graph_on_gpu(gpu_graph, graph, nb_vertices);
    printf("3\n");
    cudaMalloc((void **)&gpu_dist_array, dist_array_size);
    cudaMalloc((void **)&gpu_marked_vertex, marked_vertex_size);
    cudaMalloc((void **)&gpu_parents, parents_size);
    cudaMalloc((void **)&gpu_path, parents_size);
    printf("4\n");
    cudaEvent_t exec_start, exec_stop; // timer for execution only
    float elapsed_exec;                // elapsed time
    cudaEventCreate(&exec_start);
    cudaEventCreate(&exec_stop);

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < nb_vertices; i++)
    {
        dist_array[i] = DBL_MAX;
        marked_vertex[i] = false;
    }

    // Distance from source to itself is always 0
    dist_array[origin] = 0;
    parents[origin] = -1;

    // start distance is zero; ensures it will be first pulled out
    // gpu source        cpu source      memory size     HtD or DtH
    // cudaMemcpy(gpu_graph, graph, graph_size, cudaMemcpyHostToDevice));

    cudaMemcpy(gpu_dist_array, dist_array, dist_array_size, cudaMemcpyHostToDevice);
    cudaMemcpy(gpu_marked_vertex, marked_vertex, marked_vertex_size, cudaMemcpyHostToDevice);
    cudaMemcpy(gpu_parents, parents, parents_size, cudaMemcpyHostToDevice);

    printf("avant\n");
    cudaEventRecord(exec_start);
    for (int i = 0; i < nb_vertices; i++)
    {
        min_distance_cuda<<<gridMin, blockMin>>>(gpu_dist_array, gpu_marked_vertex, gpu_closest_vertex, gpu_nb_vertices);              // find min
        cudaRelax<<<gridRelax, blockRelax>>>(gpu_graph, gpu_dist_array, gpu_parents, gpu_marked_vertex, gpu_closest_vertex, gpu_path); // relax
        // cudaRelax(vertex_t * *graph, double *dist_array, int *parents, bool *marked_vertex, int *global_min_index, path_t *path)
    }
    cudaEventRecord(exec_stop);

    // // save data in PN, ND matrices
    // cudaMemcpy(node_dist, gpu_node_dist, data_array, cudaMemcpyDeviceToHost));
    // cudaMemcpy(parent_node, gpu_parent_node, int_array, cudaMemcpyDeviceToHost));
    // cudaMemcpy(visited_node, gpu_visited_node, int_array, cudaMemcpyDeviceToHost));

    // for (i = 0; i < VERTICES; i++) {                //record resulting parent array and node distance
    //     pn_matrix[version*VERTICES + i] = parent_node[i];
    //     dist_matrix[version*VERTICES + i] = node_dist[i];
    // }

    // free memory
    cudaFree(gpu_graph);
    cudaFree(gpu_dist_array);
    cudaFree(gpu_marked_vertex);
    cudaFree(gpu_parents);
    free_path_on_gpu(gpu_path);

    // return dist_array[destination];
    return 0.0;
}

__global__ void min_distance_cuda(double *dist, bool *marked_vertex, int *global_min_index, int *nb_vertices)
{
    double min = DBL_MAX;
    int min_index = -1;
    int i;

    for (i = 0; i < nb_vertices[0]; i++)
    {
        if ((dist[i] < min) && (marked_vertex[i] != 1))
        {
            min = dist[i];
            min_index = i;
        }
    }

    global_min_index[0] = min_index;
    marked_vertex[min_index] = 1;
}

__global__ void cudaRelax(vertex_t **graph, double *dist_array, int *parents, bool *marked_vertex, int *global_min_index, path_t *path)
{
    uint32_t next = blockIdx.x * blockDim.x + threadIdx.x; // global ID
    int source = global_min_index[0];
    bool vertex_is_in_visibilite = false;
    double current_cost;

    // data_t edge = graph[source*VERTICES + next];
    edge_t *edge = NULL;

    for (uint32_t i = 0; i < graph[source]->nb_edges_out; i++)
    {
        if (graph[source]->out_edges[i]->id == next)
        {
            edge = graph[source]->out_edges[i];
        }
    }
    if (edge == NULL)
    {
        return;
    }

    for (uint32_t i = 0; i < path->nb_visibilite; i++)
    {
        if (path->visibilite[i] == next)
        {
            vertex_is_in_visibilite = true;
        }
    }
    if (!vertex_is_in_visibilite)
    {
        return;
    }

    current_cost = ((path->profil) * (edge->dist)) + ((1 - (path->profil)) * (edge->danger));

    if (!marked_vertex[next] &&
        (current_cost < dist_array[next]))
    {
        dist_array[next] = current_cost;
        parents[next] = source;
    }
}