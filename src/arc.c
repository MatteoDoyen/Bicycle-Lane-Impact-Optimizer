#include "../header/arc.h"
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







// Function to create a new Arc
Arc *createArc(int id, double dist, double danger, Noeud *pred, Noeud *succ)
{
    Arc *newArc = (Arc *)malloc(sizeof(Arc));
    newArc->id = id;
    newArc->dist = dist;
    newArc->danger = danger;
    newArc->succ = succ;
    newArc->predecesseur = pred;
    if (succ->id == 2 || pred->id == 2)
    {
        printf("ici\n");
    }
    if(succ->nb_arc_entrant==succ->max_arc_entrant){
        succ->entrant = realloc(succ->entrant,succ->max_arc_entrant*2);
    }
    succ->entrant[succ->nb_arc_entrant++] = newArc;

    if(succ->nb_arc_sortant==succ->max_arc_sortant){
        succ->sortant = realloc(succ->sortant,succ->max_arc_sortant*2);
    }
    pred->sortant[pred->nb_arc_sortant++] = newArc;
    return newArc;
}

// Function to read CSV file and create graph
void createGraphFromCSV(const char *filename, Noeud **graph, Arc*** arcArray, int *numNoeuds, int *nbArcs)
{
    unsigned int node_i,node_j;
    int nb_col, nb_row;
    char ***csv_matrix = readCSVFile(filename, &nb_row, &nb_col, ";");
    *nbArcs = nb_row;
    // Assuming the maximum node index found is the number of node and that all node index are contiguous
    *numNoeuds=get_nb_node(csv_matrix,nb_row);
    *graph = (Noeud *)malloc(*numNoeuds * sizeof(Noeud));

    // The number of rows is the exact number of arcs in the csv file
    *arcArray = (Arc **)malloc(nb_row * sizeof(Arc*));

    //Initialize each node
    for (int i = 0; i < *numNoeuds; i++)
    {
        (*graph)[i].nb_arc_entrant = 0;
        (*graph)[i].max_arc_entrant = DEFAULT_NEIGHBOURS;
        (*graph)[i].entrant = malloc(sizeof(Arc *) * DEFAULT_NEIGHBOURS);
        (*graph)[i].nb_arc_entrant = 0;
        (*graph)[i].max_arc_sortant = DEFAULT_NEIGHBOURS;
        (*graph)[i].sortant = malloc(sizeof(Arc *) * DEFAULT_NEIGHBOURS);
        (*graph)[i].id = i;
    }

    // start at one because first line is header
    for (int i = 0; i < nb_row; i++)
    {
        node_i = atoi(csv_matrix[i][G_NODEI_INDEX]);
        node_j = atoi(csv_matrix[i][G_NODEJ_INDEX]);
        (*arcArray)[i] = createArc(
            atoi(csv_matrix[i][G_ID_INDEX]),
            atof(csv_matrix[i][G_DISTANCE_INDEX]),
            atof(csv_matrix[i][G_DANGER_INDEX]),
            &(*graph)[node_i],
            &(*graph)[node_j]);
    }
    
    freeCSVMatrix(csv_matrix, nb_row, nb_col);
}