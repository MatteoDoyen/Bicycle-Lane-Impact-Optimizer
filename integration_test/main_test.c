#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "config.h"
#include "compute_edges.h"
#include "../header/dijkstra.h"
#include "util.h"
#include "dijkstra_test.h"
#include "compute_edges_test.h"


#define CONFIG_FILE_PATH "./conf_test/conf_unit_test.json"
#define CONFIG_REDUCED_FILE_PATH "./conf_test/conf_unit_test_reduced.json"
#define CONFIG_REAL_FILE_PATH "./conf_test/conf_unit_test_real.json"

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_dijkstra_forward_vs_precomputed_data_cost);
    RUN_TEST(test_dijkstra_forward_vs_precomputed_data_path);
    RUN_TEST(test_dijkstra_forward_vs_backward_path);
    RUN_TEST(test_dijkstra_forward_vs_backward_cost);

    RUN_TEST(test_get_edges_to_optimize_for_budget_one_edge);
    RUN_TEST(test_get_edges_to_optimize_for_budget_no_budget);
    RUN_TEST(test_get_edges_to_optimize_for_budget_budget_over_max);
    RUN_TEST(test_get_edges_to_optimize_for_budget_no_edge);
    RUN_TEST(test_get_edges_to_optimize_for_budget_multiple_edges);
    RUN_TEST(test_get_edges_to_optimize_for_budget_vs_precomputed);
    return UNITY_END();
}