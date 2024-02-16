#ifndef DISPLAY_PROGRESS_H
#define DISPLAY_PROGRESS_H

#define UPDATE_INTERVAL_MS 500

typedef struct {
    long double total_budget;
    long double *budget_left;
}print_progress_args_t;

void* progress_bar_thread(void* arg);
void print_progress_bar(double progress, double total);

#endif // DISPLAY_PROGRESS_H