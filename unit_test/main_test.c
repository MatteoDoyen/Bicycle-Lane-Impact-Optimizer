#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "djikstra.h"

double calc_path_cps(double dist, double danger, double alpha)
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

void test_vertices_out_array_scaling(void){
    
    vertex_t vertices[DEFAULT_NEIGHBOURS+2];
    for (unsigned int i = 0; i < DEFAULT_NEIGHBOURS+2; i++)
    {
        
       vertices[i].nb_edges_in = 0;
       vertices[i].max_edges_in = DEFAULT_NEIGHBOURS;
       vertices[i].in_edges = calloc(1,sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
       vertices[i].nb_edges_out = 0;
       vertices[i].max_edges_out = DEFAULT_NEIGHBOURS;
       vertices[i].out_edges = calloc(1,sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
       vertices[i].id = i;

       if(i==0){
        continue;
       }
        create_edge(i,1,1,&vertices[0],&vertices[i]);
    }
    unsigned int max_id = 0;
    unsigned int current_id;
    for (unsigned int i = 0; i < vertices[0].nb_edges_out; i++)
    {
        current_id = vertices[0].out_edges[i]->succ->id;
        max_id = current_id > max_id ? current_id : max_id;
    }
    TEST_ASSERT_EQUAL_MESSAGE(DEFAULT_NEIGHBOURS+1,max_id,"probleme");
    TEST_ASSERT_EQUAL_MESSAGE(DEFAULT_NEIGHBOURS*2,vertices[0].max_edges_out,"probleme");
    for (int i = 0; i < DEFAULT_NEIGHBOURS+2; i++)
    {
        free(vertices[i].in_edges);
        free(vertices[i].out_edges);
    }
}

void test_vertices_in_array_scaling(void){
    
    vertex_t vertices[DEFAULT_NEIGHBOURS+2];
    for (unsigned int i = 0; i < DEFAULT_NEIGHBOURS+2; i++)
    {
        
       vertices[i].nb_edges_in = 0;
       vertices[i].max_edges_in = DEFAULT_NEIGHBOURS;
       vertices[i].in_edges = calloc(1,sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
       vertices[i].nb_edges_out = 0;
       vertices[i].max_edges_out = DEFAULT_NEIGHBOURS;
       vertices[i].out_edges = calloc(1,sizeof(edge_t *) * DEFAULT_NEIGHBOURS);
       vertices[i].id = i;

       if(i==0){
        continue;
       }
        create_edge(i,1,1,&vertices[i],&vertices[0]);
    }
    unsigned int max_id = 0;
    unsigned int current_id;
    for (unsigned int i = 0; i < vertices[0].nb_edges_in; i++)
    {
        current_id = vertices[0].in_edges[i]->pred->id;
        max_id = current_id > max_id ? current_id : max_id;
    }
    TEST_ASSERT_EQUAL_MESSAGE(DEFAULT_NEIGHBOURS+1,max_id,"probleme");
    TEST_ASSERT_EQUAL_MESSAGE(DEFAULT_NEIGHBOURS*2,vertices[0].max_edges_in,"probleme");
    for (int i = 0; i < DEFAULT_NEIGHBOURS+2; i++)
    {
        free(vertices[i].in_edges);
        free(vertices[i].out_edges);
    }
}

void test_djikstra_forward_vs_backward_path(void)
{
    vertex_t *graph;
    edge_t **edge_array;
    path_t *paths;
    int nb_vertices;
    int nb_paths;
    int nb_edges;

    double *dist_array_forward, *dist_array_backward;
    int *parent_array_forward, *parent_array_backward;
    int current;
    int *diff_path;

    get_graph("./data_test/data_graphe.csv",";", &graph, &edge_array, &nb_vertices, &nb_edges);

    get_paths("./data_test/data_path.csv", ";", &paths, &nb_paths);
    for (int i = 0; i < nb_paths; i++)
    {
        djikstra_forward(graph, nb_vertices, &dist_array_forward, &parent_array_forward, &paths[i]);
        djikstra_backward(graph, nb_vertices, &dist_array_backward, &parent_array_backward, &paths[i]);

        // used to get the number of element
        // when current=-1 there is no parent and the previous current is equal to the
        // destination for backward djikstra
        current = paths[i].origin;
        int vertex_nb = 0;
        do
        {
            vertex_nb++;
        } while ((current = parent_array_backward[current]) != -1);

        // we store the vertices id of the shortest path
        // found by the backward djikstra in the inverted order
        diff_path = calloc(1, sizeof(int) * vertex_nb);
        current = paths[i].origin;
        for (int x = vertex_nb - 1; x >= 0; x--)
        {
            diff_path[x] = current;
            current = parent_array_backward[current];
        }

        // We can now compare the path found by the backward djikstra
        // with the path found by the forward djikstra
        current = paths[i].destination;
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
    free_paths(paths, nb_paths);
}

void test_djikstra_forward_vs_backward_cost(void)
{
    vertex_t *graph;
    edge_t **edge_array;
    path_t *paths;
    int nb_vertices;
    int nb_edges;
    double *dist_array_forward, *dist_array_backward;
    int nb_paths;
    double cost_forward, cost_backward;
    char str[80];

    get_graph("./data_test/data_graphe.csv",";", &graph, &edge_array, &nb_vertices, &nb_edges);
    get_paths("./data_test/data_path.csv", ";", &paths, &nb_paths);

    for (int i = 0; i < nb_paths; i++)
    {
        cost_forward = djikstra_forward(graph, nb_vertices, &dist_array_forward, NULL, &paths[i]);
        cost_backward = djikstra_backward(graph, nb_vertices, &dist_array_backward, NULL, &paths[i]);

        sprintf(str, "path %d failed, incorrect cps calculated", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.001, cost_forward, cost_backward, str);

        free(dist_array_forward);
        free(dist_array_backward);
    }
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_paths(paths, nb_paths);
}

void test_djikstra_forward(void)
{

    vertex_t *graph;
    edge_t **edge_array;
    path_t *paths;
    int nb_vertices;
    int nb_edges;
    double *dist_array;
    int nb_paths;
    double cost;
    double expected_cost;
    char str[80];

    get_graph("./data_test/data_graphe.csv",";", &graph, &edge_array, &nb_vertices, &nb_edges);
    get_paths("./data_test/data_path.csv", ";", &paths, &nb_paths);

    for (int i = 0; i < nb_paths; i++)
    {
        cost = djikstra_forward(graph, nb_vertices, &dist_array, NULL, &paths[i]);
        expected_cost = calc_path_cps(paths[i].cps_djikstra_dist, paths[i].danger, paths[i].profil);

        sprintf(str, "path %d failed, incorrect cps calculated", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.000001, expected_cost, cost, str);

        free(dist_array);
    }
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_paths(paths, nb_paths);
}

void test_cost_function(void)
{
    path_t path;
    edge_t edge;

    edge.danger = 5;
    edge.dist = 2;
    path.profil = 1; // alpha

    // when the alpha is equal 1, the cost function should return the
    //  edge distance
    double return_value = cost_function(&path, &edge);
    TEST_ASSERT_EQUAL_DOUBLE(edge.dist, return_value);

    // when the alpha is equal 0.5, the cost function should return the
    //  edge distance + the edge danger divided by two
    path.profil = 0.5;
    return_value = cost_function(&path, &edge);

    // when the alpha is equal 1, the cost function should return the
    //  edge danger
    TEST_ASSERT_EQUAL_DOUBLE(3.5, return_value);
    path.profil = 0;
    return_value = cost_function(&path, &edge);
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

    TEST_ASSERT_EQUAL(-1, min_distance(dist, NULL));
    TEST_ASSERT_EQUAL(0, min_distance(dist, vertices_to_visit));
    dist[0] = 180;
    TEST_ASSERT_EQUAL(1, min_distance(dist, vertices_to_visit));
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
    RUN_TEST(test_vertices_in_array_scaling);
    RUN_TEST(test_vertices_out_array_scaling);
    return UNITY_END();
}