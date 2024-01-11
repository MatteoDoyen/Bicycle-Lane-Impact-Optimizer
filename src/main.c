#include "../header/edge.h"
#include "../header/path.h"
#include "../header/djikstra.h"
#include "../header/compute_edges.h"
#include "../header/display_progress.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define BUDGET 150
#define DATA_DIRECTORY  "./data/artificiel"
#define PATHS_FILE_NAME DATA_DIRECTORY"/data_path.csv"
#define GRAPH_FILE_NAME DATA_DIRECTORY"/data_graphe.csv"
#define RESULT_DIRECTORY "./results"
#define RESULT_FILE_NAME RESULT_DIRECTORY"/edges_to_improve_"

int main()
{
    long double budget_left;
    long double budget = 10;
    pthread_t progressBar;
    print_progress_args_t print_progress_args;
    char result_file_name[200];
    snprintf(result_file_name, sizeof(result_file_name),"%s%.2Lf.csv",RESULT_FILE_NAME,budget);

    print_progress_args.total_budget = budget;
    print_progress_args.budget_left = &budget_left;

    int rc = pthread_create(&progressBar, NULL, progressBarThread, &print_progress_args);
    if (rc) {
        printf("Error creating progress bar thread: %d\n", rc);
        return -1;
    }
    selected_edge_t *selected_edges = NULL;
    get_edges_to_optimize_for_budget_threaded(budget,&budget_left, GRAPH_FILE_NAME, PATHS_FILE_NAME,12, &selected_edges);
    budget_left = budget;
    pthread_join(progressBar, NULL);
    save_selected_edges(selected_edges,result_file_name);
    free_select_edges(selected_edges);

    return 0;
}