#ifndef EDGE_H
#define EDGE_H
#include <stdbool.h>
// Forward declaration of edge_t struct
struct edge_t;

// vertex_t struct definition
typedef struct vertex_t {
    unsigned int id;
    struct edge_t **sortant; //array of outgoing edge
    unsigned int nb_edges_out; //current number of outgoing edge
    unsigned int max_edges_out; //max number of outgoing edge
    struct edge_t **entrant; //array of incoming edge
    unsigned int nb_edges_in; //current numbr of incoming edge
    unsigned int max_edges_in; //max number of incoming edge
} vertex_t;

// edge_t struct definition
typedef struct edge_t {
    double dist;
    double danger;
    bool amenagement;
    unsigned int id;
    struct vertex_t *succ;
    struct vertex_t *pred;
} edge_t;


#define G_ID_INDEX 0
#define G_NODEI_INDEX 1
#define G_NODEJ_INDEX 2
#define G_DISTANCE_INDEX 3
#define G_DANGER_INDEX 4
#define G_AMENAGEMENT_INDEX 5

#define DEFAULT_NEIGHBOURS 20 // by experiences it seems that there is never a node with more than 20 neighbours


unsigned int get_nb_node(char*** csv_matrix, int nb_row);
edge_t *create_edge(int id, double dist, double danger, vertex_t *pred, vertex_t *succ);
void get_graph(const char *filename,char * separator, vertex_t **graph, edge_t*** edge_array, int *numvertex_ts, int *nbedge_ts);
void free_graph(vertex_t *graph,int num_vertices);
void free_edge(edge_t **edge_array,int nb_edges);

#endif //EDGE_H