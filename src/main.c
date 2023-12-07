#include "../header/arc.h"
#include <stdlib.h>

int main()
{
    Noeud *graph;
    int numNoeuds;

    createGraphFromCSV("./Data/small_data.csv", &graph, &numNoeuds);
    // printf("salut %d \n",numNoeuds);
   
    int source = 1538;  // Source vertex
    int destination = 1106;  // Destination vertex

    dijkstra(graph, source, destination, numNoeuds);

    // Free allocated memory
    free(graph);

    return 0;
}