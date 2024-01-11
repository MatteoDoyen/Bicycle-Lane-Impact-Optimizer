#include "../header/display_progress.h"
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

void* progressBarThread(void* arg) {
    // printf("entrée ?\n");
    print_progress_args_t * print_args = (print_progress_args_t *)arg;
    long double currentProgress;
    long double lastProgress = -1;
    while (1) {
        usleep(UPDATE_INTERVAL_MS * 1000); // Sleep for the specified interval
        currentProgress = *print_args->budget_left;
        if (currentProgress != lastProgress) {
            // printf("la ?\n");
            lastProgress = currentProgress;
            printProgressBar(currentProgress, print_args->total_budget);
        }

        // Check if the worker thread has completed
        if (currentProgress >= print_args->total_budget) {
            printProgressBar(print_args->total_budget, print_args->total_budget);
            break;
        }
    }
    printf("\n");
    pthread_exit(NULL);
}

void printProgressBar(double progress, double total) {
    const int barWidth = 50;
    double percentage = (double)(progress / total);
    int barLength = barWidth * percentage;

    printf("\033[1;32m[");
    for (int i = 0; i < barWidth; ++i) {
        if (i < barLength) {
            printf("=");
        } else {
            printf(" ");
        }
    }

    printf("] budget used : %3.2f%%\033[0m\r", percentage * 100);
    fflush(stdout);
}