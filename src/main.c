#include "../header/djikstra.h"
#include "../header/compute_edges_pthread.h"
#include "../header/compute_edges_omp.h"
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

    long double budget_used, copy_budget_used;
    pthread_t progressBar;
    print_progress_args_t print_progress_args;
    char result_file_name[200];
    int ret;
    cifre_conf_t config;

    if (argc != 2)
    {
        fprintf(stderr, "wrong number of arguments, usage : ./cifre $(config_file_path)\n");
        return 1;
    }

    ret = set_config(argv[1], &config);
    if (ret != OK)
    {
        return 1;
    }

    snprintf(result_file_name, sizeof(result_file_name), "%s%.2Lf.csv", RESULT_FILE_NAME, config.budget);

    print_progress_args.total_budget = config.budget;
    print_progress_args.budget_left = &budget_used;

    ret = pthread_create(&progressBar, NULL, progressBarThread, &print_progress_args);
    if (ret)
    {
        fprintf(stderr, "Error creating progress bar thread: %d\n", ret);
        return -1;
    }
    double_unsigned_list_t *selected_edges = NULL;
    clock_gettime(CLOCK_MONOTONIC, &start);
    ret = get_edges_to_optimize_for_budget_omp(&config, &budget_used, &selected_edges);
    if (ret != OK)
    {
        budget_used = config.budget;
        pthread_join(progressBar, NULL);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed_time_thread = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    copy_budget_used = budget_used;
    budget_used = config.budget;
    pthread_join(progressBar, NULL);
    save_selected_edges(selected_edges, result_file_name);
    free_double_unsigned_list_t(selected_edges);

    printf("budget used : %Lf\n", copy_budget_used);
    printf("Time taken by myThreadedFunction: %f seconds\n", elapsed_time_thread);
    printf("Result saved to : %s\n", result_file_name);

    free_config(&config);
    return 0;
}