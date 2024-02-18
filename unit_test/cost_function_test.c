#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "cost_function_test.h"
#include "../header/graph.h"
#include "../header/path.h"
#include "../header/dijkstra.h"
#include "util.h"


void test_cost_function_alpha_equal_one(void)
{
    path_t path = {.profil = 1};
    edge_t edge = {.danger = 5, .dist = 2};


    // when the alpha is equal 1, the cost function should return the
    //  edge distance
    double return_value = cost_function(path.profil, &edge.dist, &edge.danger);
    TEST_ASSERT_EQUAL_DOUBLE(edge.dist, return_value);
}

void test_cost_function_alpha_equal_zero_point_five(void)
{
    path_t path = {.profil = 0.5};
    edge_t edge = {.danger = 5, .dist = 2};

    // when the alpha is equal 0.5, the cost function should return the
    //  edge distance + the edge danger divided by two
    double return_value = cost_function(path.profil, &edge.dist, &edge.danger);
    TEST_ASSERT_EQUAL_DOUBLE(3.5, return_value);
}
void test_cost_function_alpha_equal_zero(void)
{

    path_t path = {.profil = 0};
    edge_t edge = {.danger = 5, .dist = 2};

    // when alpha equal 0 it should return the edge danger
    double return_value = cost_function(path.profil, &edge.dist, &edge.danger);
    TEST_ASSERT_EQUAL_DOUBLE(edge.danger, return_value);
}