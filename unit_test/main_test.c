#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "dijkstra_test.h"
#include "min_distance_test.h"
#include "cost_function_test.h"
#include "updated_dist_test.h"
#include "array_scaling_test.h"

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
    // RUN_TEST(test_get_edges_to_optimize_for_budget_no_edge);
    RUN_TEST(test_dijkstra_forward_one_vertex);
    RUN_TEST(test_dijkstra_forward_two_vertex);
    RUN_TEST(test_dijkstra_forward_straight_line);
    RUN_TEST(test_dijkstra_forward_complex_graph);
    RUN_TEST(test_dijkstra_forward_negative_edge);
    RUN_TEST(test_dijkstra_forward_no_path);

    RUN_TEST(test_dijkstra_backward_one_vertex);
    RUN_TEST(test_dijkstra_backward_two_vertex);
    RUN_TEST(test_dijkstra_backward_straight_line);
    RUN_TEST(test_dijkstra_backward_complex_graph);
    RUN_TEST(test_dijkstra_backward_negative_edge);
    RUN_TEST(test_dijkstra_backward_no_path);

    // RUN_TEST(test_dijkstra_backward_straight_line);
    // RUN_TEST(test_dijkstra_backward_complex_graph);
    // RUN_TEST(test_dijkstra_backward_negative_edge);
    // RUN_TEST(test_dijkstra_backward_no_path);

    RUN_TEST(test_min_distance_empty_dist_array_and_list);
    RUN_TEST(test_min_distance_empty_list);
    RUN_TEST(test_min_distance_high_value);
    RUN_TEST(test_min_distance_negative);
    RUN_TEST(test_min_distance_zero);

    RUN_TEST(test_vertices_in_array_scaling);
    RUN_TEST(test_vertices_out_array_scaling);

    RUN_TEST(test_cost_function_alpha_equal_one);
    RUN_TEST(test_cost_function_alpha_equal_zero_point_five);
    RUN_TEST(test_cost_function_alpha_equal_zero);

    RUN_TEST(test_updated_dist_zero_distance);
    RUN_TEST(test_updated_dist_zero_danger);
    RUN_TEST(test_updated_dist_max_distance);
    return UNITY_END();
}