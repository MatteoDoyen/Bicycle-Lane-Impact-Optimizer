#include "../header/edge.h"
#include "../header/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

uint32_t get_nb_node(char ***csv_matrix, uint32_t nb_row)
{
    uint32_t numNode = 0;
    uint32_t node_i, node_j;
    for (uint32_t i = 1; i < nb_row; i++)
    {
        node_i = atoi(csv_matrix[i][G_NODEI_INDEX]);
        node_j = atoi(csv_matrix[i][G_NODEJ_INDEX]);
        numNode = node_i > numNode ? node_i : numNode;
        numNode = node_j > numNode ? node_j : numNode;
    }
    return numNode + 1;
}

// Function to create a new edge_t
int create_edge(uint32_t id, edge_t **new_edge_ref, double dist, double danger, vertex_t *pred, vertex_t *succ)
{
    edge_t *newedge_t;
    *new_edge_ref = (edge_t *)calloc(1, sizeof(edge_t));
    if (*new_edge_ref == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return MEMORY_ALLOC_ERROR;
    }
    newedge_t = *new_edge_ref;
    newedge_t->id = id;
    newedge_t->dist = dist;
    newedge_t->danger = danger;
    newedge_t->succ = succ;
    newedge_t->pred = pred;
    if (succ->nb_edges_in == succ->max_edges_in)
    {
        succ->in_edges = realloc(succ->in_edges, (succ->max_edges_in * 2) * sizeof(edge_t *));
        if (succ->in_edges == NULL)
        {
            free(new_edge_ref);
            return MEMORY_REALLOC_ERROR;
        }
        succ->max_edges_in = succ->max_edges_in * 2;
    }
    succ->in_edges[succ->nb_edges_in++] = newedge_t;

    if (pred->nb_edges_out == pred->max_edges_out)
    {
        pred->out_edges = realloc(pred->out_edges, (pred->max_edges_out * 2) * sizeof(edge_t *));
        if (pred->out_edges == NULL)
        {
            free(new_edge_ref);
            return MEMORY_REALLOC_ERROR;
        }
        pred->max_edges_out = pred->max_edges_out * 2;
    }
    pred->out_edges[pred->nb_edges_out++] = newedge_t;

    return OK;
}

// Function to read CSV file and create graph
int get_graph(cifre_conf_t * config,vertex_t ***graph_ref, edge_t ***edge_array, uint32_t *num_vertices, uint32_t *nb_edges)
{
    unsigned int node_i, node_j;
    uint32_t nb_col, nb_row;
    char ***csv_matrix;
    int ret_code;
    // char ***csv_matrix = readCSVFile(filename, &nb_row, &nb_col, separator);
    ret_code = readCSVFile(config->graph_file_path, &csv_matrix, &nb_row, &nb_col, config->csv_delimiter);
    if (ret_code != OK)
    {
        return ret_code;
    }
    *nb_edges = nb_row;
    // Assuming the maximum node index found is the number of node and that all node index are contiguous
    *num_vertices = get_nb_node(csv_matrix, nb_row);

    *graph_ref = (vertex_t **)calloc(*num_vertices, sizeof(vertex_t *));
    vertex_t **graph = *graph_ref;

    // *graph = (vertex_t *)calloc(*num_vertices,sizeof(vertex_t));

    // The number of rows is the exact number of edges in the csv file
    *edge_array = (edge_t **)calloc(nb_row, sizeof(edge_t *));

    // Initialize each node
    for (uint32_t i = 0; i < *num_vertices; i++)
    {
        graph[i] = calloc(1, sizeof(vertex_t));
        graph[i]->nb_edges_in = 0;
        graph[i]->max_edges_in = DEFAULT_NEIGHBOURS;
        graph[i]->in_edges = calloc(1, sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
        graph[i]->nb_edges_out = 0;
        graph[i]->max_edges_out = DEFAULT_NEIGHBOURS;
        graph[i]->out_edges = calloc(1, sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
        graph[i]->id = i;
    }

    // start at one because first line is header
    for (uint32_t i = 0; i < nb_row; i++)
    {
        node_i = atoi(csv_matrix[i][G_NODEI_INDEX]);
        node_j = atoi(csv_matrix[i][G_NODEJ_INDEX]);

        ret_code = create_edge(
            atoi(csv_matrix[i][G_ID_INDEX]), &(*edge_array)[i],
            atof(csv_matrix[i][G_DISTANCE_INDEX]),
            atof(csv_matrix[i][G_DANGER_INDEX]),
            graph[node_i],
            graph[node_j]);

        if (ret_code != OK)
        {
            // free all the ones before;
            return ret_code;
        }
    }

    freeCSVMatrix(csv_matrix, nb_row, nb_col);
    return OK;
}

void free_graph(vertex_t **graph, int num_vertices)
{
    for (int i = 0; i < num_vertices; i++)
    {
        free(graph[i]->out_edges);
        free(graph[i]->in_edges);
        free(graph[i]);
    }
    free(graph);
}

void free_edge(edge_t **edge_array, int nb_edges)
{
    for (int i = 0; i < nb_edges; i++)
    {
        free(edge_array[i]);
    }
    free(edge_array);
}