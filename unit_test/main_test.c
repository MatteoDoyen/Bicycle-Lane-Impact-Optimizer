#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "djikstra.h"

double calc_trace_cps(double dist, double danger, double alpha)
{
    long double alpha_less = 1 - alpha;
    return (dist * alpha) + (alpha_less * danger);
}

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_djikstra_forward_vs_backward_path(void)
{
    vertex_t *graph;
    edge_t **edge_array;
    trace_t *traces;
    int nb_vertices;
    int nb_traces;
    int nb_edges;

    double *dist_array_forward, *dist_array_backward;
    int *parent_array_forward, *parent_array_backward;
    int current;
    int *diff_path;

    get_graph("./data_test/data_graphe.csv",";", &graph, &edge_array, &nb_vertices, &nb_edges);

    get_traces("./data_test/data_path.csv", ";", &traces, &nb_traces);
    for (int i = 0; i < nb_traces; i++)
    {
        djikstra_forward(graph, nb_vertices, &dist_array_forward, &parent_array_forward, &traces[i]);
        djikstra_backward(graph, nb_vertices, &dist_array_backward, &parent_array_backward, &traces[i]);

        // used to get the number of element
        // when current=-1 there is no parent and the previous current is equal to the
        // destination for backward djikstra
        current = traces[i].origin;
        int vertex_nb = 0;
        do
        {
            vertex_nb++;
        } while ((current = parent_array_backward[current]) != -1);

        // we store the vertices id of the shortes path
        // found by the backward djikstra in the inverted order
        diff_path = calloc(1, sizeof(int) * vertex_nb);
        current = traces[i].origin;
        for (int x = vertex_nb - 1; x >= 0; x--)
        {
            diff_path[x] = current;
            current = parent_array_backward[current];
        }

        // We can now compare the path found by the backward djikstra
        // with the path found by the forward djikstra
        current = traces[i].destination;
        int x = 0;
        while (current != -1 && x < vertex_nb)
        {
            TEST_ASSERT_EQUAL_MESSAGE(diff_path[x], current, "Incorrect vertex id found");
            current = parent_array_forward[current];
            x++;
        }
        free(diff_path);
        free(dist_array_forward);
        free(dist_array_backward);
        free(parent_array_backward);
        free(parent_array_forward);
    }
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_traces(traces, nb_traces);
}

void test_djikstra_forward_vs_backward_cost(void)
{
    vertex_t *graph;
    edge_t **edge_array;
    trace_t *traces;
    int nb_vertices;
    int nb_edges;
    double *dist_array_forward, *dist_array_backward;
    int nb_traces;
    double cost_forward, cost_backward;
    char str[80];

    get_graph("./data_test/data_graphe.csv",";", &graph, &edge_array, &nb_vertices, &nb_edges);
    get_traces("./data_test/data_path.csv", ";", &traces, &nb_traces);

    for (int i = 0; i < nb_traces; i++)
    {
        cost_forward = djikstra_forward(graph, nb_vertices, &dist_array_forward, NULL, &traces[i]);
        cost_backward = djikstra_backward(graph, nb_vertices, &dist_array_backward, NULL, &traces[i]);

        sprintf(str, "trace %d failed, incorrect cps calculated", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.001, cost_forward, cost_backward, str);

        free(dist_array_forward);
        free(dist_array_backward);
    }
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_traces(traces, nb_traces);
}

void test_djikstra_forward(void)
{

    vertex_t *graph;
    edge_t **edge_array;
    trace_t *traces;
    int nb_vertices;
    int nb_edges;
    double *dist_array;
    int nb_traces;
    double cost;
    double expected_cost;
    char str[80];

    get_graph("./data_test/data_graphe.csv",";", &graph, &edge_array, &nb_vertices, &nb_edges);
    get_traces("./data_test/data_path.csv", ";", &traces, &nb_traces);

    for (int i = 0; i < nb_traces; i++)
    {
        cost = djikstra_forward(graph, nb_vertices, &dist_array, NULL, &traces[i]);
        expected_cost = calc_trace_cps(traces[i].cps_djikstra_dist, traces[i].danger, traces[i].profil);

        sprintf(str, "trace %d failed, incorrect cps calculated", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.000001, expected_cost, cost, str);

        free(dist_array);
    }
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_traces(traces, nb_traces);
}

void test_cost_function(void)
{
    trace_t trace;
    edge_t edge;

    edge.danger = 5;
    edge.dist = 2;
    trace.profil = 1; // alpha

    // when the alpha is equal 1, the cost function should return the
    //  edge distance
    double return_value = cost_function(&trace, &edge);
    TEST_ASSERT_EQUAL_DOUBLE(edge.dist, return_value);

    // when the alpha is equal 0.5, the cost function should return the
    //  edge distance + the edge danger divided by two
    trace.profil = 0.5;
    return_value = cost_function(&trace, &edge);

    // when the alpha is equal 1, the cost function should return the
    //  edge danger
    TEST_ASSERT_EQUAL_DOUBLE(3.5, return_value);
    trace.profil = 0;
    return_value = cost_function(&trace, &edge);
    TEST_ASSERT_EQUAL_DOUBLE(edge.danger, return_value);
}

void test_min_distance(void)
{

    double dist[4];
    for (int i = 0; i < 4; i++)
    {
        dist[i] = i;
    }

    list_node_t vertices_to_visit[4];
    for (int i = 0; i < 3; i++)
    {
        vertices_to_visit[i].vertex_id = i;
        vertices_to_visit[i].next = (vertices_to_visit + (i + 1));
    }
    vertices_to_visit[3].vertex_id = 3;
    vertices_to_visit[3].next = NULL;

    TEST_ASSERT_EQUAL(-1, minDistance(dist, NULL));
    TEST_ASSERT_EQUAL(0, minDistance(dist, vertices_to_visit));
    dist[0] = 180;
    TEST_ASSERT_EQUAL(1, minDistance(dist, vertices_to_visit));
}

// not needed when using generate_test_runner.rb
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_cost_function);
    RUN_TEST(test_min_distance);
    RUN_TEST(test_djikstra_forward);
    RUN_TEST(test_djikstra_forward_vs_backward_path);
    RUN_TEST(test_djikstra_forward_vs_backward_cost);
    return UNITY_END();
}