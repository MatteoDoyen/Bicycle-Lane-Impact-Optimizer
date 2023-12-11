#include "../header/arc.h"
#include "../header/trace.h"
#include <stdlib.h>

typedef struct {
    unsigned int id;
    double distance;
}result_t;

#define BUDGET_MAX 15

int main()
{
    Noeud *graph;
    Arc** arcArray;
    int nbNoeuds;
    int nbArc;

    createGraphFromCSV("./data/artificiel/data_graphe.csv", &graph, &arcArray, &nbNoeuds,&nbArc);
    // printf("salut %d \n",numNoeuds);
   
    // int source = 1538;  // Source vertex
    // int destination = 1106;  // Destination vertex

    // dijkstra(graph, source, destination, nbNoeuds);


    result_t * resultat = malloc(sizeof(result_t)*nbArc);
    unsigned int result_index = 0;
    
    int trace_nb;
    double cost;
    double budget= 0;
    

    trace_t *traces = get_traces("./data/artificiel/data_path.csv", ";", &trace_nb);

    for (int trace_id = 0; trace_id < trace_nb; trace_id++)
    {
        traces[trace_id].djikstra_dist = djikstra(graph,traces[trace_id].origin,traces[trace_id].destination,nbNoeuds);
    }


    for (int path_id = 0; path_id < trace_nb; path_id++)
    {

        // if path is not already optimised
        if (arcArray[path_id]->danger != arcArray[path_id]->dist){
            
            //optimize the path so that we can test its impact on the algorithm
            arcArray[path_id]->danger = arcArray[path_id]->dist;
            for (int trace_id = 0; trace_id < trace_nb; trace_id++)
            {
                if(budget>BUDGET_MAX){
                    printf("budget max\n");
                    return 0;
                }
                if (tronconIsInTrace(&traces[trace_id], arcArray[path_id]))
                {
                    cost = djikstra(graph,traces[trace_id].origin,traces[trace_id].destination,nbNoeuds);
                    // printf("dedans %f\n",cost);
                    if(cost < traces[trace_id].djikstra_dist){
                        printf("inf  %f\n",traces[trace_id].djikstra_dist-cost);
                        traces[trace_id].djikstra_dist = cost;
                        resultat[result_index].id = path_id;
                        resultat[result_index].distance = arcArray[path_id]->dist;
                        budget+=arcArray[path_id]->dist;
                        result_index++;
                    }
                    break;
                }
            }
        }
        else{
            // next path
            continue;
        }
    }
    printf("sortie\n");
     for (unsigned int i = 0; i < result_index; i++)
    {
        printf("%d %f\n",resultat[result_index].id,resultat[result_index].distance);
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