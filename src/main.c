#include "../header/edge.h"
#include "../header/path.h"
#include "../header/djikstra.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define BUDGET 150
#define PATHS_FILE_NAME "./data/artificiel/data_path.csv"
#define GRAPH_FILE_NAME "./data/artificiel/data_graphe.csv"


int main()
{
   struct timespec start, end;
   double elapsed_time_mono;
   double elapsed_time_thread;

    for (int i = 10; i <= 20; i++)
    {
        selected_edge_t * selected_edges;
        // printf("budget : %d\n",i);
        clock_gettime(CLOCK_MONOTONIC, &start);
        get_edges_to_optimize_for_budget(i,GRAPH_FILE_NAME,PATHS_FILE_NAME,&selected_edges);
        clock_gettime(CLOCK_MONOTONIC, &end);
        free_select_edges(selected_edges);
        elapsed_time_mono = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

        clock_gettime(CLOCK_MONOTONIC, &start);
        get_edges_to_optimize_for_budget_threaded(i,GRAPH_FILE_NAME,PATHS_FILE_NAME,12,&selected_edges);
        print_selected_edges(selected_edges);
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsed_time_thread = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        printf("%d %f %f\n",i,elapsed_time_thread,elapsed_time_mono);
        // printf("\n");
        // printf("%d\n",i);
        free_select_edges(selected_edges);
    }
    
    // Print the result
    // printf("Time taken by myThreadedFunction: %f seconds\n", elapsed_time);
    
    return 0;
}