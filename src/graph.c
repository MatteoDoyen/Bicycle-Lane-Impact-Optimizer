#include "../header/graph.h"
#include "../header/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

/*
 * Function: get_nb_node
 * ----------------------------------
 * This function calculates the number of nodes in a graph based on a CSV matrix.
 *
 * Parameters:
 * - csv_matrix: a pointer to a 2D array of strings representing the CSV matrix
 * - nb_row: the number of rows in the CSV matrix
 * - config: a pointer to a config_t struct containing graph configuration information
 *
 * Returns:
 * - The number of nodes in the graph.
 *
 * Note:
 * - The function assumes that the CSV matrix is properly formatted and that the node indices in the matrix are valid.
 * - The function adds 1 to the maximum node index found in the matrix to account for zero-based indexing.
 *
 * Example usage:
 * - uint32_t numNodes = get_nb_node(csv_matrix, nb_row, &config);
 */
uint32_t get_nb_node(char ***csv_matrix, uint32_t nb_row, config_t *config)
{
    uint32_t numNode = 0;
    uint32_t node_i, node_j;
    for (uint32_t i = 1; i < nb_row; i++)
    {
        node_i = atoi(csv_matrix[i][config->graph_indexes.node_i]);
        node_j = atoi(csv_matrix[i][config->graph_indexes.node_j]);
        numNode = node_i > numNode ? node_i : numNode;
        numNode = node_j > numNode ? node_j : numNode;
    }
    return numNode + 1;
}

/*
 * Function: create_edge
 * ----------------------------------
 * This function creates a new edge with the given parameters and adds it to the graph.
 *
 * Parameters:
 * - id: The ID of the new edge.
 * - new_edge_ref: A pointer to a pointer to the new edge.
 * - dist: The distance of the new edge.
 * - danger: The danger level of the new edge.
 * - pred: The predecessor vertex of the new edge.
 * - succ: The successor vertex of the new edge.
 *
 * Returns:
 * - OK if the edge creation is successful.
 * - MEMORY_ALLOC_ERROR if memory allocation fails.
 * - MEMORY_REALLOC_ERROR if memory reallocation fails.
 *
 * Note:
 * - The function dynamically allocates memory for the new edge and updates the predecessor and successor vertices accordingly.
 * - It is the responsibility of the caller to free the memory allocated for the new edge when it is no longer needed.
 *
 * Example usage:
 * - edge_t *new_edge;
 *   int result = create_edge(1, &new_edge, 10.5, 0.8, vertex1, vertex2);
 *   if (result == OK) {
 *       // Edge creation successful
 *   } else {
 *       // Error occurred
 *   }
 */
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
    
    // Reallocate memory for the out_edges array if the number of out edges exceeds the maximum
    if (succ->nb_edges_in == succ->max_edges_in)
    {
        // multiply by 2 to avoid reallocating memory too often
        succ->in_edges = realloc(succ->in_edges, (succ->max_edges_in * 2) * sizeof(edge_t *));
        if (succ->in_edges == NULL)
        {
            free(new_edge_ref);
            return MEMORY_REALLOC_ERROR;
        }
        succ->max_edges_in = succ->max_edges_in * 2;
    }
    succ->in_edges[succ->nb_edges_in++] = newedge_t;

    // Reallocate memory for the in_edges array if the number of out edges exceeds the maximum
    if (pred->nb_edges_out == pred->max_edges_out)
    {
        // multiply by 2 to avoid reallocating memory too often
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

/*
 * Function: get_graph
 * ----------------------------------
 * This function reads a CSV file containing graph data and constructs a graph structure.
 *
 * Parameters:
 * - config: a pointer to the configuration structure
 * - graph: a pointer to the graph structure to be constructed
 *
 * Returns:
 * - OK if the graph is successfully constructed
 * - ERROR if there is an error reading the CSV file or constructing the graph
 *
 * Note:
 * - The function assumes that the maximum node index found in the CSV file is the number of nodes in the graph,
 *   and that all node indices are contiguous.
 * - The function dynamically allocates memory for the graph structure and its components.
 * - The function frees the memory allocated for the CSV matrix after constructing the graph.
 *
 * Example usage:
 * - int ret_code = get_graph(&config, &graph);
 */
int get_graph(config_t *config, graph_t *graph)
{
    unsigned int node_i, node_j;
    uint32_t nb_col, nb_row;
    char ***csv_matrix;
    int ret_code;

    // Read the CSV file
    ret_code = read_csv_file(config->graph_file_path, &csv_matrix, &nb_row, &nb_col, config->csv_delimiter);
    if (ret_code != OK)
    {
        return ret_code;
    }

    // Set the number of edges in the graph
    graph->nb_edges = nb_row;

    // Assuming the maximum node index found is the number of nodes and that all node indices are contiguous
    graph->nb_vertices = get_nb_node(csv_matrix, nb_row, config);

    // Allocate memory for the vertex array
    graph->vertex_array = (vertex_t **)calloc(graph->nb_vertices, sizeof(vertex_t *));

    // Allocate memory for the edge array
    graph->edge_array = (edge_t **)calloc(nb_row, sizeof(edge_t *));

    // Initialize each node
    for (uint32_t i = 0; i < graph->nb_vertices; i++)
    {
        graph->vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph->vertex_array[i]->nb_edges_in = 0;
        graph->vertex_array[i]->max_edges_in = DEFAULT_NEIGHBOURS;
        graph->vertex_array[i]->in_edges = calloc(1, sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
        graph->vertex_array[i]->nb_edges_out = 0;
        graph->vertex_array[i]->max_edges_out = DEFAULT_NEIGHBOURS;
        graph->vertex_array[i]->out_edges = calloc(1, sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
        graph->vertex_array[i]->id = i;
    }

    // Create edges
    for (uint32_t i = 0; i < nb_row; i++)
    {
        node_i = atoi(csv_matrix[i][config->graph_indexes.node_i]);
        node_j = atoi(csv_matrix[i][config->graph_indexes.node_j]);

        ret_code = create_edge(
            atoi(csv_matrix[i][config->graph_indexes.edge_id]), &graph->edge_array[i],
            atof(csv_matrix[i][config->graph_indexes.distance]),
            atof(csv_matrix[i][config->graph_indexes.danger]),
            graph->vertex_array[node_i],
            graph->vertex_array[node_j]);

        if (ret_code != OK)
        {
            // Free the memory allocated before returning
            free_vertex_array(graph->vertex_array, graph->nb_vertices);
            if(i>0){
                free_edge(graph->edge_array, i);
            }
            free_csv_matrix(csv_matrix, nb_row, nb_col);
            return ret_code;
        }
    }

    // Free the memory allocated for the CSV matrix
    free_csv_matrix(csv_matrix, nb_row, nb_col);

    return OK;
}

/*
 * Function: free_graph
 * ----------------------------------
 * This function frees the memory allocated for a graph.
 *
 * Parameters:
 * - graph: a pointer to the graph to be freed
 *
 * Returns:
 * - None
 *
 * Note:
 * - Make sure to call this function before returning.
 *
 * Example usage:
 * - free_graph(my_graph);
 */
void free_graph(graph_t *graph)
{
    free_vertex_array(graph->vertex_array, graph->nb_vertices);
    free_edge(graph->edge_array, graph->nb_edges);
}

/*
 * Function: free_vertex_array
 * ----------------------------------
 * This function frees the memory allocated for the vertex array in a graph.
 *
 * Parameters:
 * - graph: a pointer to the vertex array to be freed
 * - num_vertices: the number of vertices in the graph
 *
 * Returns:
 * - None
 *
 * Note:
 * - Make sure to call this function before returning.
 *
 * Example usage:
 * - free_vertex_array(graph->vertex_array, graph->nb_vertices);
 */
void free_vertex_array(vertex_t **graph, int num_vertices)
{
    for (int i = 0; i < num_vertices; i++)
    {
        free(graph[i]->out_edges);
        free(graph[i]->in_edges);
        free(graph[i]);
    }
    free(graph);
}

/*
 * Function: free_edge
 * ----------------------------------
 * This function frees the memory allocated for an array of edge_t structures.
 *
 * Parameters:
 * - edge_array: The array of edge_t structures to be freed.
 * - nb_edges: The number of elements in the edge_array.
 *
 * Returns:
 * - None
 *
 * Note:
 * - This function should be called to free the memory allocated for an array of edge_t structures.
 *   It is important to call this function before returning from the program to avoid memory leaks.
 *
 * Example usage:
 * - free_edge(edge_array, nb_edges);
 */
void free_edge(edge_t **edge_array, int nb_edges)
{
    for (int i = 0; i < nb_edges; i++)
    {
        free(edge_array[i]);
    }
    free(edge_array);
}