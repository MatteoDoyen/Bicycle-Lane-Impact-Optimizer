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
// Function to find the vertex with the minimum distance value
int minDistance(double dist[], bool markedVertex[], int V)
{
    double min = DBL_MAX;
    int min_index = -1;

    for (int v = 0; v < V; v++)
    {
        if ((markedVertex[v] == false) && dist[v] < min)
        {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Dijkstra's algorithm to find the shortest path between two vertices
void dijkstra(struct Noeud *graph, int src, int dest, int V)
{
    double *dist = (double *)malloc(V * sizeof(double));
    bool *markedVertex = (bool *)malloc(V * sizeof(bool));
    int *parent = (int *)malloc(V * sizeof(int));
    int dest_vertex_id;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < V; i++)
    {
        dist[i] = DBL_MAX;
        markedVertex[i] = false;
    }

    // Distance from source to itself is always 0
    dist[src] = 0;

    // Parent of source is itself
    parent[src] = -1;

    // Find the shortest path for all vertices
    for (int count = 0; count < V - 1; count++)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        int u = minDistance(dist, markedVertex, V);
        if (u == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[u] = true;
        // Update dist value of the adjacent vertices
        for (unsigned int arc_i = 0; arc_i < graph[u].nb_arc_sortant; arc_i++)
        {
            dest_vertex_id = graph[u].sortant[arc_i]->succ->id;
            if (!markedVertex[dest_vertex_id] && graph[u].sortant[arc_i] != NULL && (dist[u] + graph[u].sortant[arc_i]->dist < dist[dest_vertex_id]))
            {
                dist[dest_vertex_id] = dist[u] + graph[u].sortant[arc_i]->dist;
                parent[dest_vertex_id] = u;
            }
        }
    }

    // Print the shortest path from source to destination
    printf("Shortest Path from %d to %d:\n", src, dest);
    int current = dest;
    while (current != -1)
    {
        printf("%d <- ", current);
        current = parent[current];
    }
    printf("\n");

    // Print the total distance of the shortest path
    printf("Total Distance: %f\n", dist[dest]);

    free(dist);
    free(markedVertex);
    free(parent);
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
    succ->entrant[succ->nb_arc_entrant++] = newArc;
    pred->sortant[pred->nb_arc_sortant++] = newArc;
    return newArc;
}

// Function to read CSV file and create graph
void createGraphFromCSV(const char *filename, Noeud **graph, int *numNoeuds)
{
    unsigned int node_i,node_j;
    int nb_col, nb_row;
    char ***csv_matrix = readCSVFile(filename, &nb_row, &nb_col, ";");
    *numNoeuds=get_nb_node(csv_matrix,nb_row);
    // Assuming a maximum of 1600 nodes, adjust as needed
    *graph = (Noeud *)malloc(*numNoeuds * sizeof(Noeud));
    for (int i = 0; i < *numNoeuds; i++)
    {
        (*graph)[i].nb_arc_entrant = 0;
        (*graph)[i].entrant = malloc(sizeof(Arc *) * 20);
        (*graph)[i].nb_arc_entrant = 0;
        (*graph)[i].sortant = malloc(sizeof(Arc *) * 20);
        (*graph)[i].id = i;
    }

    // start at one because first line is header
    for (int i = 0; i < nb_row; i++)
    {
        node_i = atoi(csv_matrix[i][G_NODEI_INDEX]);
        node_j = atoi(csv_matrix[i][G_NODEJ_INDEX]);
        createArc(
            atoi(csv_matrix[i][G_ID_INDEX]),
            atof(csv_matrix[i][G_DISTANCE_INDEX]),
            atof(csv_matrix[i][G_DANGER_INDEX]),
            &(*graph)[node_i],
            &(*graph)[node_j]);
    }
    
    freeCSVMatrix(csv_matrix, nb_row, nb_col);
}