#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "config.h"
#include "min_distance_test.h"
#include "compute_edges.h"
#include "../header/dijkstra.h"
#include "util.h"


void test_min_distance_negative(void)
{

    double dist[4];
    for (int i = 0; i < 4; i++)
    {
        dist[i] = -(i+1);
    }

    unsigned_list_t vertices_to_visit[4];
    for (int i = 0; i < 3; i++)
    {
        vertices_to_visit[i].u_value = i;
        vertices_to_visit[i].next = (vertices_to_visit + (i + 1));
    }
    vertices_to_visit[3].u_value = 3;
    vertices_to_visit[3].next = NULL;

    TEST_ASSERT_EQUAL(3, min_distance(dist, vertices_to_visit));
}

void test_min_distance_empty_list(void)
{

    double dist[4];
    for (int i = 0; i < 4; i++)
    {
        dist[i] = i;
    }
    TEST_ASSERT_EQUAL(-1, min_distance(dist, NULL));
}
void test_min_distance_empty_dist_array_and_list(void)
{

    TEST_ASSERT_EQUAL(-1, min_distance(NULL, NULL));
}

void test_min_distance_zero(void)
{

    double dist[4];
    for (int i = 0; i < 4; i++)
    {
        dist[i] = i;
    }

    unsigned_list_t vertices_to_visit[4];
    for (int i = 0; i < 3; i++)
    {
        vertices_to_visit[i].u_value = i;
        vertices_to_visit[i].next = (vertices_to_visit + (i + 1));
    }
    vertices_to_visit[3].u_value = 3;
    vertices_to_visit[3].next = NULL;

    TEST_ASSERT_EQUAL(0, min_distance(dist, vertices_to_visit));
}

bool definitelyLessThan(float a, float b, float epsilon)
{
    return (b - a) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}


void test_min_distance_high_value(void)
{

    double dist[4];
    for (int i = 0; i < 4; i++)
    {
        dist[i] = DBL_MAX;
    }
    dist[0] = DBL_MAX-1;
    double a = dist[0];
    double b = DBL_MAX;
    if(definitelyLessThan(a,b,0.1)){
        printf("DBL_MAX\n");
    }

    unsigned_list_t vertices_to_visit[4];
    for (int i = 0; i < 3; i++)
    {
        vertices_to_visit[i].u_value = i;
        vertices_to_visit[i].next = (vertices_to_visit + (i + 1));
    }
    vertices_to_visit[3].u_value = 3;
    vertices_to_visit[3].next = NULL;

    TEST_ASSERT_EQUAL(0, min_distance(dist, vertices_to_visit));
}