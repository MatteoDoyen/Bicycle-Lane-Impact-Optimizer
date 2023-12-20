#include "../header/edge.h"
#include "../header/util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>




unsigned int get_nb_node(char*** csv_matrix, int nb_row){
    unsigned int numNode=0;
    unsigned int node_i,node_j;
    for (int i = 0; i < nb_row; i++)
    {
        node_i = atoi(csv_matrix[i][G_NODEI_INDEX]);
        node_j = atoi(csv_matrix[i][G_NODEJ_INDEX]);
        numNode =  node_i > numNode ? node_i : numNode;
        numNode =  node_j > numNode ? node_j : numNode;
    }
    return numNode+1;
}

// Function to create a new edge_t
edge_t *create_edge(int id, double dist, double danger, vertex_t *pred, vertex_t *succ)
{
    edge_t *newedge_t = (edge_t *)calloc(1,sizeof(edge_t));
    newedge_t->id = id;
    newedge_t->dist = dist;
    newedge_t->danger = danger;
    newedge_t->succ = succ;
    newedge_t->pred = pred;
    if(succ->nb_edges_in==succ->max_edges_in){
        fprintf(stderr,"edges in max\n");
        succ->in_edges = realloc(succ->in_edges,(succ->max_edges_in*2)*sizeof(edge_t *));
        succ->max_edges_in = succ->max_edges_in*2;
    }
    succ->in_edges[succ->nb_edges_in++] = newedge_t;

    if(pred->nb_edges_out==pred->max_edges_out){
        pred->out_edges = realloc(pred->out_edges,(pred->max_edges_out*2)*sizeof(edge_t *));
        pred->max_edges_out = pred->max_edges_out*2;
    }
    pred->out_edges[pred->nb_edges_out++] = newedge_t;

    return newedge_t;
}

// Function to read CSV file and create graph
void get_graph(const char *filename,char * separator, vertex_t **graph, edge_t*** edge_array, int *numvertex_ts, int *nbedge_ts)
{
    unsigned int node_i,node_j;
    int nb_col, nb_row;
    char ***csv_matrix = readCSVFile(filename, &nb_row, &nb_col, separator);
    *nbedge_ts = nb_row;
    // Assuming the maximum node index found is the number of node and that all node index are contiguous
    *numvertex_ts=get_nb_node(csv_matrix,nb_row);
    *graph = (vertex_t *)calloc(1,*numvertex_ts * sizeof(vertex_t));

    // The number of rows is the exact number of edges in the csv file
    *edge_array = (edge_t **)calloc(1,nb_row * sizeof(edge_t*));

    //Initialize each node
    for (int i = 0; i < *numvertex_ts; i++)
    {
        (*graph)[i].nb_edges_in = 0;
        (*graph)[i].max_edges_in = DEFAULT_NEIGHBOURS;
        (*graph)[i].in_edges = calloc(1,sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
        (*graph)[i].nb_edges_out = 0;
        (*graph)[i].max_edges_out = DEFAULT_NEIGHBOURS;
        (*graph)[i].out_edges = calloc(1,sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
        (*graph)[i].id = i;
    }

    // start at one because first line is header
    for (int i = 0; i < nb_row; i++)
    {
        node_i = atoi(csv_matrix[i][G_NODEI_INDEX]);
        node_j = atoi(csv_matrix[i][G_NODEJ_INDEX]);
        (*edge_array)[i] = create_edge(
            atoi(csv_matrix[i][G_ID_INDEX]),
            atof(csv_matrix[i][G_DISTANCE_INDEX]),
            atof(csv_matrix[i][G_DANGER_INDEX]),
            &(*graph)[node_i],
            &(*graph)[node_j]);
    }
    
    freeCSVMatrix(csv_matrix, nb_row, nb_col);
}

void free_graph(vertex_t *graph, int numvertex_ts){
    for (int i = 0; i < numvertex_ts; i++)
    {
        free(graph[i].out_edges);
        free(graph[i].in_edges);
    }
    free(graph);
}

void free_edge(edge_t **edge_array,int nbedge_ts){
    for (int i = 0; i < nbedge_ts; i++)
    {
        free(edge_array[i]);
    }
    free(edge_array);
}