#include "../header/display_progress.h"
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

/*
 * Function: progress_bar_thread
 * ----------------------------------
 * This function is a thread function that displays a progress bar based on the current progress and total budget.
 *
 * Parameters:
 * - arg: A pointer to the print_progress_args_t structure containing the necessary arguments for printing the progress bar.
 *
 * Returns:
 * - None
 *
 * Note:
 * This function continuously checks the current progress and updates the progress bar accordingly. 
 * It exits the thread when the current progress reaches or exceeds the total budget.
 *
 * Example usage:
 * pthread_t thread;
 * print_progress_args_t args;
 * // Initialize args
 * pthread_create(&thread, NULL, progress_bar_thread, (void*)&args);
 * // Wait for the thread to complete
 * pthread_join(thread, NULL);
 */
void* progress_bar_thread(void* arg) {
    print_progress_args_t * print_args = (print_progress_args_t *)arg;
    long double current_progress;
    long double last_progress = -1;
    while (1) {

        // Sleep for the specified interval
        usleep(UPDATE_INTERVAL_MS * 1000); 
        current_progress = *print_args->budget_left;

        if (current_progress != last_progress) {
            last_progress = current_progress;
            print_progress_bar(current_progress, print_args->total_budget);
        }

        // Check if the worker thread has completed
        if (current_progress >= print_args->total_budget) {
            print_progress_bar(print_args->total_budget, print_args->total_budget);
            break;
        }
    }
    printf("\n");
    pthread_exit(NULL);
}

/*
 * Function: print_progress_bar
 * ----------------------------------
 * This function prints a progress bar based on the current progress and total budget.
 *
 * Parameters:
 * - progress: The current progress.
 * - total: The total budget.
 *
 * Returns:
 * - None
 *
 * Note:
 * This function calculates the percentage of progress, determines the length of the progress bar,
 * and prints the progress bar with the corresponding percentage.
 * It uses ANSI escape codes to format the output.
 * The displayed percentage is rounded to two decimal places.
 * The function does not return any value.
 *
 * Example usage:
 * print_progress_bar(50, 100);
 */
void print_progress_bar(double progress, double total) {
    const int bar_width = 50;
    double percentage = (double)(progress / total);
    int bar_length = bar_width * percentage;
    double displayed_percentage;

    printf("\033[1;32m[");
    for (int i = 0; i < bar_width; ++i) {
        if (i < bar_length) {
            printf("=");
        } else {
            printf(" ");
        }
    }
    // if the percentage is negative, we display 0
    displayed_percentage = (percentage * 100) > 0 ? (percentage * 100) :0; 
    printf("] budget used : %3.2f%%\033[0m\r", displayed_percentage);
    fflush(stdout);
}