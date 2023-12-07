#include "../header/arc.h"
#include "../header/trace.h"
#include <stdlib.h>

int main()
{
    Noeud *graph;
    Arc** arcArray;
    int nbNoeuds;
    int nbArc;

    createGraphFromCSV("./data/artificiel/data_graphe.csv", &graph, &arcArray, &nbNoeuds,&nbArc);
    // printf("salut %d \n",numNoeuds);
   
    int source = 1538;  // Source vertex
    int destination = 1106;  // Destination vertex

    dijkstra(graph, source, destination, nbNoeuds);

    
    int trace_nb;
    trace_t *traces = get_traces("./data/artificiel/data_path.csv", ";", &trace_nb);
    for (int path_id = 0; path_id < trace_nb; path_id++)
    {

        // if path is not already optimised
        if (arcArray[path_id]->danger != arcArray[path_id]->dist){
            
            //optimize the path so that we can test its impact on the algorithm
            arcArray[path_id]->danger = arcArray[path_id]->dist;
            for (int trace_id = 0; trace_id < trace_nb; trace_id++)
            {
                if (tronconIsInTrace(&traces[trace_id], arcArray[path_id]))
                {
                    //compute the new cost with the djikstra path
                    // printf("in trace\n");
                    break;
                }
            }
        }
        else{
            // next path
            continue;
        }
    }
    // Free allocated memory
    free(graph);
    // TODO => free arc array

    return 0;
}

/*
- deux djikstras par trace
- 

*/