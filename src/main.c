#include "../header/edge.h"
#include "../header/path.h"
#include "../header/djikstra.h"
#include <stdlib.h>
#include <stdbool.h>

#define BUDGET 3

int main()
{
    get_edges_to_optimize_for_budget(BUDGET);
    return 0;
}