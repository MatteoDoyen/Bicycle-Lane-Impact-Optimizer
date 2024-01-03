#include "../header/edge.h"
#include "../header/path.h"
#include "../header/djikstra.h"
#include "../header/compute_edges.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define BUDGET 150
#define PATHS_FILE_NAME "./data_path.csv"
#define GRAPH_FILE_NAME "./data_graphe.csv"

int main()
{
    struct timespec start, end;
    double elapsed_time_mono;
    double elapsed_time_thread;

    // for (int i = 13; i <= 20; i++)
    // {
    //     selected_edge_t * selected_edges=NULL;
    //     clock_gettime(CLOCK_MONOTONIC, &start);
    //     get_edges_to_optimize_for_budget_threaded(400,GRAPH_FILE_NAME,PATHS_FILE_NAME,i,&selected_edges);
    //     clock_gettime(CLOCK_MONOTONIC, &end);
    //     free_select_edges(selected_edges);
    //     elapsed_time_thread = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    //     printf("%i %f\n",budget,elapsed_time_thread);
    // }

    // for (int i = 100; i <= 400; i+=100)
    // {
    //     int budget = i == 400 ? 414 : i;
    //     selected_edge_t * selected_edges=NULL;
    //     // printf("budget : %d\n",i);
    //     // clock_gettime(CLOCK_MONOTONIC, &start);
    //     // get_edges_to_optimize_for_budget(budget,GRAPH_FILE_NAME,PATHS_FILE_NAME,&selected_edges);
    //     // clock_gettime(CLOCK_MONOTONIC, &end);
    //     // free_select_edges(selected_edges);
    //     // elapsed_time_mono = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    //     clock_gettime(CLOCK_MONOTONIC, &start);
    //     get_edges_to_optimize_for_budget_threaded(budget,GRAPH_FILE_NAME,PATHS_FILE_NAME,12,&selected_edges);
    //     clock_gettime(CLOCK_MONOTONIC, &end);
    //     free_select_edges(selected_edges);
    //     elapsed_time_thread = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    //     // printf("%i %f %f\n",budget,elapsed_time_thread,elapsed_time_mono);
    //     printf("%i %f\n",budget,elapsed_time_thread);
    // }
    
    selected_edge_t *selected_edges = NULL;
    clock_gettime(CLOCK_MONOTONIC, &start);
    // get_edges_to_optimize_for_budget_threaded(0.2, GRAPH_FILE_NAME, PATHS_FILE_NAME,8, &selected_edges);
    get_edges_to_optimize_for_budget_threaded(0.2, GRAPH_FILE_NAME, PATHS_FILE_NAME,12, &selected_edges);
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time_mono = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("%f %f\n",0.2,elapsed_time_mono);
    print_selected_edges(selected_edges);
    free_select_edges(selected_edges);

    // Print the result
    // printf("Time taken by myThreadedFunction: %f seconds\n", elapsed_time);

    return 0;
}