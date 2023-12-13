#ifndef ARC_H
#define ARC_H
#include <stdbool.h>
// Forward declaration of Arc struct
struct Arc;

// Noeud struct definition
typedef struct Noeud {
    unsigned int id;
    struct Arc **sortant; //array of outgoing edge
    unsigned int nb_arc_sortant; //current number of outgoing edge
    unsigned int max_arc_sortant; //max number of outgoing edge
    struct Arc **entrant; //array of incoming edge
    unsigned int nb_arc_entrant; //current numbr of incoming edge
    unsigned int max_arc_entrant; //max number of incoming edge
} Noeud;

// Arc struct definition
typedef struct Arc {
    double dist;
    double danger;
    bool amenagement;
    unsigned int id;
    struct Noeud *succ;
    struct Noeud *predecesseur;
} Arc;


#define G_ID_INDEX 0
#define G_NODEI_INDEX 1
#define G_NODEJ_INDEX 2
#define G_DISTANCE_INDEX 3
#define G_DANGER_INDEX 4
#define G_AMENAGEMENT_INDEX 5

#define DEFAULT_NEIGHBOURS 20 // by experiences it seems that there is never a node with more than 20 neighbours


unsigned int get_nb_node(char*** csv_matrix, int nb_row);
Arc *createArc(int id, double dist, double danger, Noeud *pred, Noeud *succ);
void createGraphFromCSV(const char *filename, Noeud **graph,Arc*** arcArray, int *numNoeuds, int * nbArcs);

#endif