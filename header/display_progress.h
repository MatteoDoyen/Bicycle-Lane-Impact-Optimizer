#ifndef DISPLAY_PROGRESS_H
#define DISPLAY_PROGRESS_H

#define UPDATE_INTERVAL_MS 500

typedef struct {
    long double total_budget;
    long double *budget_left;
}print_progress_args_t;

void* progressBarThread(void* arg);
void printProgressBar(int progress, int total);

#endif // DISPLAY_PROGRESS_H