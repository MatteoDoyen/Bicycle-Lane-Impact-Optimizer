#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "config.h"
#include "dijkstra_test.h"
#include "compute_edges.h"
#include "../header/dijkstra.h"
#include "util.h"

#define CONFIG_FILE_PATH "./conf_test/conf_unit_test.json"
#define CONFIG_REDUCED_FILE_PATH "./conf_test/conf_unit_test_reduced.json"
#define CONFIG_REAL_FILE_PATH "./conf_test/conf_unit_test_real.json"

double calc_path_cps(double dist, double danger, double alpha)
{
    long double alpha_less = 1 - alpha;
    return (dist * alpha) + (alpha_less * danger);
}

void test_dijkstra_forward_vs_backward_path(void)
{
    graph_t graph;
    path_t **paths;
    config_t config;
    get_config(CONFIG_FILE_PATH,&config);
    uint32_t nb_paths;
    int ret_code;
    double cost;

    double *dist_array_forward, *dist_array_backward;
    int *parent_array_forward, *parent_array_backward;
    int current,old_current;

    get_graph(&config, &graph);
    ret_code = get_paths(&config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        return;
    }

    parent_array_forward = calloc(graph.nb_vertices,sizeof(int));
    parent_array_backward = calloc(graph.nb_vertices,sizeof(int));
    dist_array_forward = calloc(graph.nb_vertices, sizeof(double));
    dist_array_backward = calloc(graph.nb_vertices, sizeof(double));

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        cost = dijkstra_bidirectional(&graph, &dist_array_forward, parent_array_forward, paths[i],true);
        dijkstra_bidirectional(&graph, &dist_array_backward, parent_array_backward, paths[i],false);
        // used to get the number of element
        // when current=-1 there is no parent and the previous current is equal to the
        // destination for backward dijkstra
        current = paths[i]->origin;

        while (current != -1)
        {
            old_current = current;
            current = parent_array_backward[current];

            if (current != -1)
            {
                TEST_ASSERT_EQUAL_MESSAGE(old_current, parent_array_forward[current], "Incorrect vertex id found");
            }
            else{
                TEST_ASSERT_EQUAL_MESSAGE(cost,dist_array_forward[old_current],"the distance to the destionation should be the same than the dijkstra cost");
            }
        }
    }
    free(parent_array_backward);
    free(parent_array_forward);
    free(dist_array_forward);
    free(dist_array_backward);
    free_graph(&graph);
    free_paths(paths, nb_paths);
    free_config(&config);
}

void test_dijkstra_forward_vs_backward_cost(void)
{
    graph_t graph;
    path_t **paths;
    config_t config;
    int ret_code;
    uint32_t nb_paths;
    double *dist_array_forward, *dist_array_backward;
    double cost_forward, cost_backward;
    char str[80];

    get_config(CONFIG_FILE_PATH,&config);
    get_graph(&config, &graph);
    ret_code = get_paths(&config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        return;
    }

    dist_array_forward = calloc(graph.nb_vertices, sizeof(double));
    dist_array_backward = calloc(graph.nb_vertices, sizeof(double));

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        cost_forward = dijkstra_bidirectional(&graph, &dist_array_forward, NULL, paths[i],true);
        cost_backward = dijkstra_bidirectional(&graph,&dist_array_backward, NULL, paths[i],false);

        sprintf(str, "path %d failed, incorrect cps precomputed", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.001, cost_forward, cost_backward, str);

    }
    free(dist_array_forward);
    free(dist_array_backward);
    free_graph(&graph);
    free_paths(paths, nb_paths);
    free_config(&config);
}

void test_dijkstra_forward_vs_precomputed_data_cost(void)
{

    graph_t graph;
    path_t **paths;
    config_t config;
    get_config(CONFIG_FILE_PATH,&config);
    uint32_t nb_paths;
    double *dist_array;
    double cost, expected_cost;
    char str[80];

    get_graph(&config, &graph);
    get_paths(&config, &paths, &nb_paths);
    
    dist_array = calloc(graph.nb_vertices, sizeof(double));
    for (uint32_t i = 0; i < nb_paths; i++)
    {
        cost = dijkstra_bidirectional(&graph, &dist_array, NULL, paths[i],true);
        expected_cost = calc_path_cps(paths[i]->cps_dijkstra_dist, paths[i]->cps_dijkstra_danger, paths[i]->profil);

        sprintf(str, "path %d failed, incorrect cps precomputed", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.000001, expected_cost, cost, "str");
    }
    free(dist_array);
    free_graph(&graph);
    free_paths(paths, nb_paths);
    free_config(&config);
}

void test_dijkstra_forward_vs_precomputed_data_path(void)
{

    graph_t graph;
    path_t **paths;
    config_t config;
    get_config(CONFIG_FILE_PATH,&config);
    int ret_code;
    uint32_t nb_paths;
    double *dist_array;
    int *parents_array;
    int current;
    int32_t vertex_index;
    char str[120];

    get_graph(&config, &graph);
    
    ret_code = get_paths(&config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        return;
    }
    parents_array = calloc(graph.nb_vertices,sizeof(int));
    dist_array = calloc(graph.nb_vertices, sizeof(double));
    for (uint32_t path_id = 0; path_id < nb_paths; path_id++)
    {
        sprintf(str, "path %d failed", path_id);
        dijkstra_bidirectional(&graph,&dist_array, parents_array, paths[path_id],true);
        current = paths[path_id]->destination;
        vertex_index = paths[path_id]->nb_dijkstra_sp-1;
        while(current!=-1 && (vertex_index >= 0)){
            TEST_ASSERT_EQUAL_MESSAGE(current, paths[path_id]->dijkstra_sp[vertex_index], str);
            current = parents_array[current];
            vertex_index--;
        }
        sprintf(str, "path %d failed, The dijsktra shortest path found is shorter or longer than the real one", path_id);
        TEST_ASSERT_EQUAL_MESSAGE(vertex_index, current, str);
    }
    free(parents_array); 
    free(dist_array);
    free_graph(&graph);
    free_paths(paths, nb_paths);
    free_config(&config);
}