#include "../header/dijkstra.h"
#include <pthread.h>
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
    struct timespec start, end;
    double elapsed_time_thread;
    double total_saved_cost;

    long double budget_used, copy_budget_used;
    pthread_t progressBar;
    print_progress_args_t print_progress_args;
    char result_file_name[200];
    int ret;
    config_t config;

    if (argc != 2)
    {
        fprintf(stderr, "wrong number of arguments, usage : ./cifre $(config_file_path)\n");
        return 1;
    }

    ret = get_config(argv[1], &config);
    if (ret != OK)
    {
        return 1;
    }

    snprintf(result_file_name, sizeof(result_file_name), "%s%.2Lf.csv", RESULT_FILE_NAME, config.budget);

    print_progress_args.total_budget = config.budget;
    print_progress_args.budget_left = &budget_used;

    ret = pthread_create(&progressBar, NULL, progress_bar_thread, &print_progress_args);
    if (ret)
    {
        fprintf(stderr, "Error creating progress bar thread: %d\n", ret);
        return -1;
    }
    double_unsigned_list_t *selected_edges = NULL;
    clock_gettime(CLOCK_MONOTONIC, &start);
    ret = get_edges_to_optimize_for_budget(&config, &budget_used, &selected_edges);
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (ret != OK)
    {
        budget_used = config.budget;
        pthread_join(progressBar, NULL);
        return 1;
    }
    elapsed_time_thread = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    copy_budget_used = budget_used;
    budget_used = config.budget;
    pthread_join(progressBar, NULL);
    total_saved_cost = get_total_saved_cost(selected_edges);
    save_selected_edges(selected_edges,RESULT_DIRECTORY, result_file_name);
    free_double_unsigned_list_t(selected_edges);

    printf("Budget used         : %.4Lf/%.4Lf\n", copy_budget_used,config.budget);
    printf("Time taken          : %f seconds\n", elapsed_time_thread);
    printf("Total saved cost    : %f\n", total_saved_cost);
    printf("Result saved to     : %s\n", result_file_name);

    free_config(&config);
    return 0;
}