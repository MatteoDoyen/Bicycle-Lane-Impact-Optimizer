#ifndef GRAPH_H
#define GRAPH_H
#include "../header/config.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct{
    double distance;
    double danger;
}vertex_cuda_t;

// Forward declaration of edge_t struct
struct edge_t;

// vertex_t struct definition
typedef struct vertex_t {
    unsigned int id;
    struct edge_t **out_edges; //array of outgoing edge
    unsigned int nb_edges_out; //current number of outgoing edge
    unsigned int max_edges_out; //max number of outgoing edge
    struct edge_t **in_edges; //array of incoming edge
    unsigned int nb_edges_in; //current numbr of incoming edge
    unsigned int max_edges_in; //max number of incoming edge
} vertex_t;

// edge_t struct definition
typedef struct edge_t {
    double dist;
    double danger;
    bool amenagement;
    uint32_t id;
    struct vertex_t *succ;
    struct vertex_t *pred;
} edge_t;

typedef struct graph_t {
    vertex_t **vertex_array;
    edge_t **edge_array;
    uint32_t nb_vertices;
    uint32_t nb_edges;
}graph_t;



#define DEFAULT_NEIGHBOURS 20 // by experiences it seems that there is rarely ever a node with more than 20 neighbors

edge_t* get_edge_if_exist(vertex_t ** graph,uint32_t vertex_source, uint32_t vertex_dest);
uint32_t get_nb_node(char*** csv_matrix, uint32_t nb_row,config_t * config);
int create_edge(uint32_t id,edge_t **new_edge_ref, double dist, double danger, vertex_t *pred, vertex_t *succ);
int get_graph(config_t *config, graph_t* graph);
void free_vertex_array(vertex_t **graph,int num_vertices);
void free_graph(graph_t * graph);
void free_edge(edge_t **edge_array,int nb_edges);

#endif //EDGE_H