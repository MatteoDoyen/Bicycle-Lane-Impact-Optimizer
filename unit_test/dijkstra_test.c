#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "dijkstra_test.h"
#include "../header/dijkstra.h"

void test_dijkstra_forward_no_path(void)
{
    // Create a graph
    graph_t graph;
    graph.nb_vertices = 3;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path with no possible route
    path_t path;
    path.origin = 0;
    path.destination = 2;

    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_forward function
    double cost = dijkstra_forward(&graph, &dist_array, parents, &path);

    int expected_parents[] = {-1, 0, 0};
    double expected_dist_array [] = {0, DBL_MAX, DBL_MAX};
    // Check the results
    TEST_ASSERT_EQUAL_DOUBLE(DBL_MAX, cost);
    TEST_ASSERT_EQUAL_INT32_ARRAY_MESSAGE(expected_parents, parents, graph.nb_vertices,"parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}

void test_dijkstra_forward_one_vertex(void){
     // Create a graph
    graph_t graph;
    graph.nb_vertices = 1;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 0.5;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;
    path.origin = 0;
    path.destination = 1;

    // Create edges
    double expected_cost=0;
 
    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_forward function
    double cost = dijkstra_forward(&graph, &dist_array, parents, &path);

    // Check the results
    int expected_parents[] = {-1};
    double expected_dist_array[] = {0};
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected_cost, cost, "cost is not correct");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_parents, parents, graph.nb_vertices, "parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}

void test_dijkstra_forward_two_vertex(void){
    // Create a graph
    graph_t graph;
    graph.nb_vertices = 2;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 0.5;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;
    path.origin = 0;
    path.destination = 1;

    double dist_edge_array[] = {2.5};
    double danger_edge_array[] = {0.8};
    // Create edges
    uint32_t nb_edges = 1;
    edge_t **edges = calloc(1, sizeof(edge_t *));
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        edges[i] = calloc(1, sizeof(edge_t));
        create_edge(i, &edges[i], dist_edge_array[i], danger_edge_array[i], graph.vertex_array[i], graph.vertex_array[i + 1]);
    }
    double expected_cost=0;
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        expected_cost += (dist_edge_array[i] * path.profil) + (1 - path.profil) * danger_edge_array[i];
    }


    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_forward function
    double cost = dijkstra_forward(&graph, &dist_array, parents, &path);

    // Check the results
    int expected_parents[] = {-1, 0};
    double expected_dist_array[] = {0, 1.65};
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected_cost, cost, "cost is not correct");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_parents, parents, graph.nb_vertices, "parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        free(edges[i]);
    }
    free(edges);
    
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}

void test_dijkstra_forward_straight_line(void)
{
    // Create a graph
    graph_t graph;
    graph.nb_vertices = 5;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 0;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;
    path.origin = 0;
    path.destination = 4;

    double dist_edge_array[] = {2.5, 1.5, 3.0, 2.0};
    double danger_edge_array[] = {0.8, 0.6, 0.9, 0.7};
    // Create edges
    uint32_t nb_edges = 4;
    edge_t **edges = calloc(4, sizeof(edge_t *));
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        edges[i] = calloc(1, sizeof(edge_t));
        create_edge(i, &edges[i], dist_edge_array[i], danger_edge_array[i], graph.vertex_array[i], graph.vertex_array[i + 1]);
    }
    double expected_cost=0;
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        expected_cost += (dist_edge_array[i] * path.profil) + (1 - path.profil) * danger_edge_array[i];
    }


    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_forward function
    double cost = dijkstra_forward(&graph, &dist_array, parents, &path);

    // Check the results
    int expected_parents[] = {-1, 0, 1, 2, 3};
    double expected_dist_array[] = {0, 0.8, 1.4, 2.3, 3};
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected_cost, cost, "cost is not correct");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_parents, parents, graph.nb_vertices, "parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        free(edges[i]);
    }
    free(edges);
    
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}

void test_dijkstra_forward_complex_graph(void)
{
    uint32_t nb_vertex = 10;
    double dist[10][10] = {
        {-1, 1.2, -1, -1, 0.5, -1, -1, -1, -1, -1},
        {1.2, -1, 2.5, -1, -1, -1, -1, -1, -1, -1},
        {-1, 2.5, -1, 3.8, -1, -1, -1, -1, -1, -1},
        {-1, -1, 3.8, -1, 4.1, -1, -1, -1, -1, -1},
        {0.5, -1, -1, 4.1, -1, 5.4, -1, -1, -1, -1},
        {-1, -1, -1, -1, 5.4, -1, 6.7, -1, -1, -1},
        {-1, -1, -1, -1, -1, 6.7, -1, 7.9, -1, -1},
        {-1, -1, -1, -1, -1, -1, 7.9, -1, 8.2, -1},
        {-1, -1, -1, -1, -1, -1, -1, 8.2, -1, 9.5},
        {-1, -1, -1, -1, -1, -1, -1, -1, 9.5, -1} 
    };
    // Create a graph
    graph_t graph;
    graph.nb_vertices = nb_vertex;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 0;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;
    path.origin = 0;
    path.destination = 8;

    // Create edges
    uint32_t nb_edges = 20;
    edge_t **edges = calloc(nb_edges, sizeof(edge_t *));
    for (uint32_t i = 0; i < nb_vertex; i++)
    {
        for (uint32_t j = 0; j < nb_vertex; j++)
        {
            if(dist[i][j] > 0.0){
                edges[i] = calloc(1, sizeof(edge_t));
                create_edge(i, &edges[i], dist[i][j], dist[i][j], graph.vertex_array[i], graph.vertex_array[j]);
            }
        }        
    }
    double expected_cost=28.7;

    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_forward function
    double cost = dijkstra_forward(&graph, &dist_array, parents, &path);
    int expected_parents[] = {-1, 0, 1, 4, 0, 4, 5,6,7,8};
    double expected_dist_array[] =  {0, 1.2, 3.7, 4.6, 0.5, 5.9,12.6,20.5,28.7,38.2};
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected_cost, cost, "cost is not correct");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_parents,parents, graph.nb_vertices, "parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        free(edges[i]);
    }
    free(edges);
    
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(path.visibilite);
    free(graph.vertex_array);
}

void test_dijkstra_forward_negative_edge(void)
{
    // Create a graph
    graph_t graph;
    graph.nb_vertices = 3;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 1;
    path.origin = 0;
    path.destination = 2;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;

    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Create a negative edge
    edge_t *negative_edge = calloc(1, sizeof(edge_t));
    negative_edge->dist = -5.0;
    negative_edge->danger = 0.5;
    negative_edge->succ = graph.vertex_array[2];

    // Add the negative edge to the graph
    graph.vertex_array[0]->nb_edges_out = 1;
    graph.vertex_array[0]->out_edges = calloc(1, sizeof(edge_t *));
    graph.vertex_array[0]->out_edges[0] = negative_edge;

    // Call the dijkstra_forward function
    dijkstra_forward(&graph, &dist_array, parents, &path);

    // Check the results
    double expected_dist_array[] = {0, DBL_MAX, DBL_MAX};
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices, "dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    free(negative_edge);
    free(path.visibilite);
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}


/*
    DIJKSTRA BACKWARD
*/


void test_dijkstra_backward_no_path(void)
{
    // Create a graph
    graph_t graph;
    graph.nb_vertices = 3;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path with no possible route
    path_t path;
    path.origin = 0;
    path.destination = 2;

    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_forward function
    double cost = dijkstra_backward(&graph, &dist_array, parents, &path);

    int expected_parents[] = {0, 0, -1};
    double expected_dist_array [] = {DBL_MAX, DBL_MAX, 0};
    // Check the results
    TEST_ASSERT_EQUAL_DOUBLE(DBL_MAX, cost);
    TEST_ASSERT_EQUAL_INT32_ARRAY_MESSAGE(expected_parents, parents, graph.nb_vertices,"parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}

void test_dijkstra_backward_one_vertex(void){
     // Create a graph
    graph_t graph;
    graph.nb_vertices = 1;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 0.5;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;
    path.origin = 1;
    path.destination = 0;

    // Create edges
    double expected_cost=0;
 
    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_backward function
    double cost = dijkstra_backward(&graph, &dist_array, parents, &path);

    // Check the results
    int expected_parents[] = {-1};
    double expected_dist_array[] = {0};
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected_cost, cost, "cost is not correct");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_parents, parents, graph.nb_vertices, "parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}

void test_dijkstra_backward_two_vertex(void){
    // Create a graph
    graph_t graph;
    graph.nb_vertices = 2;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 0.5;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;
    path.origin = 0;
    path.destination = 1;

    double dist_edge_array[] = {2.5};
    double danger_edge_array[] = {0.8};
    // Create edges
    uint32_t nb_edges = 1;
    edge_t **edges = calloc(1, sizeof(edge_t *));
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        edges[i] = calloc(1, sizeof(edge_t));
        create_edge(i, &edges[i], dist_edge_array[i], danger_edge_array[i], graph.vertex_array[i], graph.vertex_array[i + 1]);
    }
    double expected_cost=0;
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        expected_cost += (dist_edge_array[i] * path.profil) + (1 - path.profil) * danger_edge_array[i];
    }


    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_backward function
    double cost = dijkstra_backward(&graph, &dist_array, parents, &path);

    // Check the results
    int expected_parents[] = {1, -1};
    double expected_dist_array[] = {1.65,0};
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected_cost, cost, "cost is not correct");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_parents, parents, graph.nb_vertices, "parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        free(edges[i]);
    }
    free(edges);
    
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}

void test_dijkstra_backward_straight_line(void)
{
    // Create a graph
    graph_t graph;
    graph.nb_vertices = 5;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 0;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;
    path.origin = 0;
    path.destination = 4;

    double dist_edge_array[] = {2.5, 1.5, 3.0, 2.0};
    double danger_edge_array[] = {0.8, 0.6, 0.9, 0.7};
    // Create edges
    uint32_t nb_edges = 4;
    edge_t **edges = calloc(4, sizeof(edge_t *));
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        edges[i] = calloc(1, sizeof(edge_t));
        create_edge(i, &edges[i], dist_edge_array[i], danger_edge_array[i], graph.vertex_array[i], graph.vertex_array[i + 1]);
    }
    double expected_cost=0;
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        expected_cost += (dist_edge_array[i] * path.profil) + (1 - path.profil) * danger_edge_array[i];
    }


    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_backward function
    double cost = dijkstra_backward(&graph, &dist_array, parents, &path);

    // Check the results
    int expected_parents[] = {1, 2, 3, 4, -1};
    double expected_dist_array[] = {3, 2.2, 1.6, 0.7, 0};
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected_cost, cost, "cost is not correct");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_parents, parents, graph.nb_vertices, "parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        free(edges[i]);
    }
    free(edges);
    
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}

void test_dijkstra_backward_complex_graph(void)
{
    uint32_t nb_vertex = 10;
    double dist[10][10] = {
        {-1, 1.2, -1, -1, 0.5, -1, -1, -1, -1, -1},
        {1.2, -1, 2.5, -1, -1, -1, -1, -1, -1, -1},
        {-1, 2.5, -1, 3.8, -1, -1, -1, -1, -1, -1},
        {-1, -1, 3.8, -1, 4.1, -1, -1, -1, -1, -1},
        {0.5, -1, -1, 4.1, -1, 5.4, -1, -1, -1, -1},
        {-1, -1, -1, -1, 5.4, -1, 6.7, -1, -1, -1},
        {-1, -1, -1, -1, -1, 6.7, -1, 7.9, -1, -1},
        {-1, -1, -1, -1, -1, -1, 7.9, -1, 8.2, -1},
        {-1, -1, -1, -1, -1, -1, -1, 8.2, -1, 9.5},
        {-1, -1, -1, -1, -1, -1, -1, -1, 9.5, -1} 
    };
    // Create a graph
    graph_t graph;
    graph.nb_vertices = nb_vertex;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 0;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;
    path.origin = 0;
    path.destination = 8;

    // Create edges
    uint32_t nb_edges = 20;
    edge_t **edges = calloc(nb_edges, sizeof(edge_t *));
    for (uint32_t i = 0; i < nb_vertex; i++)
    {
        for (uint32_t j = 0; j < nb_vertex; j++)
        {
            if(dist[i][j] > 0.0){
                edges[i] = calloc(1, sizeof(edge_t));
                create_edge(i, &edges[i], dist[i][j], dist[i][j], graph.vertex_array[i], graph.vertex_array[j]);
            }
        }        
    }
    double expected_cost=28.7;

    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Call the dijkstra_forward function
    double cost = dijkstra_backward(&graph, &dist_array, parents, &path);
    int expected_parents[] = {4,0,1,4,5,6,7,8,-1,8};
    double expected_dist_array[] =  {28.7,29.9,32.4,32.3,28.2,22.8,16.1,8.2,0,9.5};
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expected_cost, cost, "cost is not correct");
    TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_parents,parents, graph.nb_vertices, "parents array is not correct");
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices,"dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    for (uint32_t i = 0; i < nb_edges; i++)
    {
        free(edges[i]);
    }
    free(edges);
    
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(path.visibilite);
    free(graph.vertex_array);
}

void test_dijkstra_backward_negative_edge(void)
{
    // Create a graph
    graph_t graph;
    graph.nb_vertices = 3;
    graph.vertex_array = calloc(graph.nb_vertices, sizeof(vertex_t *));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        graph.vertex_array[i] = calloc(1, sizeof(vertex_t));
        graph.vertex_array[i]->id = i;
        graph.vertex_array[i]->nb_edges_out = 0;
        graph.vertex_array[i]->out_edges = NULL;
    }

    // Create a path
    path_t path;
    path.profil = 1;
    path.origin = 0;
    path.destination = 2;
    path.visibilite = calloc(graph.nb_vertices,sizeof(uint32_t));
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        path.visibilite[i] = i;
    }
    path.nb_visibilite = graph.nb_vertices;

    // Create distance array and parents array
    double *dist_array = calloc(graph.nb_vertices, sizeof(double));
    int *parents = calloc(graph.nb_vertices, sizeof(int));

    // Create a negative edge
    edge_t *negative_edge = calloc(1, sizeof(edge_t));
    negative_edge->dist = -5.0;
    negative_edge->danger = 0.5;
    negative_edge->succ = graph.vertex_array[2];

    // Add the negative edge to the graph
    graph.vertex_array[0]->nb_edges_out = 1;
    graph.vertex_array[0]->out_edges = calloc(1, sizeof(edge_t *));
    graph.vertex_array[0]->out_edges[0] = negative_edge;

    // Call the dijkstra_backward function
    dijkstra_backward(&graph, &dist_array, parents, &path);

    // Check the results
    double expected_dist_array[] = {DBL_MAX, DBL_MAX, 0};
    TEST_ASSERT_EQUAL_DOUBLE_ARRAY_MESSAGE(expected_dist_array, dist_array, graph.nb_vertices, "dist array is not correct");

    // Clean up
    free(dist_array);
    free(parents);
    free(negative_edge);
    free(path.visibilite);
    for (uint32_t i = 0; i < graph.nb_vertices; i++)
    {
        free(graph.vertex_array[i]);
    }
    free(graph.vertex_array);
}