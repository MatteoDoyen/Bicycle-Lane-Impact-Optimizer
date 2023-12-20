#include "../header/edge.h"
#include "../header/path.h"
#include "../header/djikstra.h"
#include <stdlib.h>
#include <stdbool.h>

#define BUDGET 3
#define PATHS_FILE_NAME "./data/artificiel/data_path.csv"
#define GRAPH_FILE_NAME "./data/artificiel/data_graphe.csv"


int main()
{
    // get_edges_to_optimize_for_budget(BUDGET,GRAPH_FILE_NAME,PATHS_FILE_NAME);
    get_edges_to_optimize_for_budget_threaded(BUDGET,GRAPH_FILE_NAME,PATHS_FILE_NAME,12);
    return 0;
}