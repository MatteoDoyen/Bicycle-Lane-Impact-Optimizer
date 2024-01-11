#include "../header/djikstra.h"
#include "../header/compute_edges.h"
#include "../header/display_progress.h"
#include "../header/config.h"
#include "../header/util.h"
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define RESULT_DIRECTORY "./results"
#define RESULT_FILE_NAME RESULT_DIRECTORY "/edges_to_improve_"

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "wrong number of arguments, usage : ./cifre $(config_file_path)\n");
        return 1;
    }
    int ret;
    cifre_conf_t config;
    ret = set_config(argv[1], &config);
    if (ret != OK)
    {
        return 1;
    }
    long double budget_used;
    pthread_t progressBar;
    print_progress_args_t print_progress_args;
    char result_file_name[200];
    snprintf(result_file_name, sizeof(result_file_name), "%s%.2Lf.csv", RESULT_FILE_NAME, config.budget);

    print_progress_args.total_budget = config.budget;
    print_progress_args.budget_left = &budget_used;

    int rc = pthread_create(&progressBar, NULL, progressBarThread, &print_progress_args);
    if (rc)
    {
        fprintf(stderr, "Error creating progress bar thread: %d\n", rc);
        return -1;
    }
    selected_edge_t *selected_edges = NULL;
    ret = get_edges_to_optimize_for_budget_threaded(&config, &budget_used, &selected_edges);
    if (ret != OK)
    {
        return 1;
    }
    budget_used = config.budget;
    pthread_join(progressBar, NULL);
    save_selected_edges(selected_edges, result_file_name);
    free_select_edges(selected_edges);
    printf("Result saved to : %s\n", result_file_name);
    free_config(&config);
    return 0;
}