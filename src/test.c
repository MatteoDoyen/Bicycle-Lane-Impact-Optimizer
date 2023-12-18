#include <stdio.h>
#include <stdlib.h>
#include "../header/djikstra.h"

enum naturel
{
    OK,
    ERROR_CODE
};

double calc_trace_cps(double dist, double danger, double alpha){
    long double alpha_less = 1-alpha;
    return (dist*alpha) + (alpha_less * danger);
}


int test_djikstra_cpc_trace(void)
{
    Noeud *graph;
    Arc **arcArray;
    int nbNoeuds;
    int nbArc;

    createGraphFromCSV("./data/artificiel/data_graphe.csv", &graph, &arcArray, &nbNoeuds, &nbArc);

    double *dist_array;
    int *parent_array;
    int nbTrace;
    unsigned int current;
    // int *diff_path;
    double cout;
    // while (current != -1)
    // {
    //     printf("%d <- ", current);
    //     current = parent[current];
    // }

    trace_t *traces = get_traces("./data/artificiel/data_path.csv", ";", &nbTrace);
    for (int i = 0; i < nbTrace; i++)
    {
        current = traces[i].destination;
        cout = djikstra_test(graph, nbNoeuds, &dist_array,&parent_array, &traces[i],true);
        
        for (unsigned int x = traces[i].nb_djikstra_sp -1; x > 0; x--)
        {
            if(traces[i].djikstra_sp[x]!=current){
                // diff_path = malloc(sizeof(unsigned int)*traces[i].nb_djikstra_sp);

                printf("trace_id : %d id_cpc %d id_calc_cpc %d cout %f\n",i,traces[i].djikstra_sp[x],current,cout);
                
                int current = traces[i].destination;
                // int z = traces[i].nb_djikstra_sp-1; 
                while (current != -1)
                {
                    // diff_path[z] = current;
                    printf("%d <- ", current);
                    current = parent_array[current];
                    // z--;
                }
                // for (unsigned int j = 0; j < traces[i].nb_djikstra_sp; j++)
                // {
                //     printf("%d -> ", diff_path[j]);
                // }
                // free(diff_path);
                printf("trace %d %f %f\n",i,cout,calc_trace_cps(traces[i].cps_djikstra_dist,traces[i].danger,traces[i].profil));
                if(calc_trace_cps(traces[i].cps_djikstra_dist,traces[i].danger,traces[i].profil)>(cout+0.2)){
                    return ERROR_CODE;
                    break;
                }
                
            }
            else{
                current = parent_array[current];
            }
            
        }
        // printf("broke \n");
        printf("trace %d %f %f\n",i,cout,calc_trace_cps(traces[i].cps_djikstra_dist,traces[i].danger,traces[i].profil));
        free(parent_array);
        
    }
    return OK;
}

int test_djikstra_trace(void)
{
    Noeud *graph;
    Arc **arcArray;
    int nbNoeuds;
    int nbArc;

    createGraphFromCSV("./data/artificiel/data_graphe.csv", &graph, &arcArray, &nbNoeuds, &nbArc);

    double *dist_array;
    int *parent_array;
    int nbTrace;
    double dist_djikstra = 0;

    trace_t *traces = get_traces("./data/artificiel/data_path.csv", ";", &nbTrace);
    for (int i = 1; i < nbTrace; i++)
    {
        dist_djikstra = djikstra_test(graph, nbNoeuds, &dist_array,&parent_array, &traces[i],true);
        if (dist_djikstra != traces[i].cps_djikstra_dist)
        {
            printf("trace %d\n",i);
            printf("        origin djikstra     :  %f\n",traces[i].cps_djikstra_dist);
            printf("        calculated djikstra :  %f\n",dist_djikstra);
            return ERROR_CODE;
        }
    }
    return OK;
}

int main()
{
    // test_djikstra_trace();
    test_djikstra_cpc_trace();
    return 0;
}
