#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "config.h"
#include "updated_dist_test.h"
#include "compute_edges.h"
#include "../header/dijkstra.h"
#include "util.h"

void test_updated_dist_zero_distance(void)
{
    // Create an edge
    edge_t edge;
    edge.pred = malloc(sizeof(vertex_t));
    edge.succ = malloc(sizeof(vertex_t));
    edge.succ->id = 0;
    edge.pred->id = 1;
    edge.dist = 0.0;
    edge.danger = 4.0;

    // Create distance arrays
    double forward_dijkstra[] = {0, 2.0};
    double backward_dijkstra[] = {1.5, 0};

    // Call the updated_dist function
    double result = updated_dist(&edge, forward_dijkstra, backward_dijkstra);

    // Check the result
    // No matter the profile, the distance replace the danger in the call to the cost function
    TEST_ASSERT_EQUAL_DOUBLE(3.5, result);

    // Clean up
    free(edge.pred);
    free(edge.succ);
}

void test_updated_dist_zero_danger(void)
{
    // Create an edge
    edge_t edge;
    edge.pred = malloc(sizeof(vertex_t));

    edge.succ = malloc(sizeof(vertex_t));
    edge.succ->id = 0;
    edge.pred->id = 1;
    edge.dist = 3.0;
    edge.danger = 0;

    // Create distance arrays
    double forward_dijkstra[] = {0, 2.0};
    double backward_dijkstra[] = {1.5, 0};

    // Call the updated_dist function
    double result = updated_dist(&edge, forward_dijkstra, backward_dijkstra);

    // Check the result
    TEST_ASSERT_EQUAL_DOUBLE(6.5, result);

    // Clean up
    free(edge.pred);
    free(edge.succ);
}

void test_updated_dist_max_distance(void)
{
    // Create an edge
    edge_t edge;
    edge.pred = malloc(sizeof(vertex_t));
    edge.succ = malloc(sizeof(vertex_t));
    edge.succ->id = 0;
    edge.pred->id = 1;
    edge.dist = DBL_MAX;

    // Create distance arrays
    double forward_dijkstra[] = {0, 1.5};
    double backward_dijkstra[] = {1.5, 0};

    // Call the updated_dist function
    double result = updated_dist(&edge, forward_dijkstra, backward_dijkstra);

    // Check the result
    TEST_ASSERT_EQUAL_DOUBLE(DBL_MAX, result);

    // Clean up
    free(edge.pred);
    free(edge.succ);
}