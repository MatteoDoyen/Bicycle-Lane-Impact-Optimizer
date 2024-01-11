#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "config.h"
#include "compute_edges.h"
#include "util.h"

#define CONFIG_FILE_PATH "./conf_test/conf_unit_test.json"
#define CONFIG_REDUCED_FILE_PATH "./conf_test/conf_unit_test_reduced.json"

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

void test_get_edges_to_optimize_for_budget_thread_vs_single(void)
{
    long double budget_left;
    cifre_conf_t config;
    set_config(CONFIG_FILE_PATH,&config);
    selected_edge_t *temp_single, *temp_thread;
    selected_edge_t *selected_edges_single = NULL;
    selected_edge_t *selected_edges_multi = NULL;

    get_edges_to_optimize_for_budget(&config,&budget_left,&selected_edges_single);
    get_edges_to_optimize_for_budget_threaded(&config,&budget_left, &selected_edges_multi);

    temp_single = selected_edges_single;
    temp_thread = selected_edges_multi;
    while (temp_single != NULL)
    {
        // TEST_ASSERT_EQUAL_MESSAGE(expexted_id[taille],temp->edge_id,"Only one edge must be selected for the budget");
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(temp_single->cost_saved, temp_thread->cost_saved, "Both cost saved should be equal");
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(temp_single->edge_id, temp_thread->edge_id, "Both edge_id should be the same");
        temp_single = temp_single->next;
        temp_thread = temp_thread->next;
    }
    TEST_ASSERT_EQUAL_MESSAGE(temp_single, temp_thread, "Both pointers should be equals to NULL");
    free_select_edges(selected_edges_single);
    free_select_edges(selected_edges_multi);
}

void test_get_edges_to_optimize_for_budget_no_edge(void)
{

    cifre_conf_t config;
    set_config(CONFIG_REDUCED_FILE_PATH,&config);
    config.budget = 0.49;
    long double budget_left; // lowest dist to optimize is 0.5
    int taille = 0;
    selected_edge_t *selected_edges;

    get_edges_to_optimize_for_budget(&config,&budget_left,&selected_edges);

    selected_edge_t *temp = selected_edges;
    while (temp != NULL)
    {
        temp = temp->next;
        taille++;
    }
    TEST_ASSERT_EQUAL_MESSAGE(0, taille, "Only two edge must be selected for the budget");
    free_select_edges(selected_edges);
}

void test_get_edges_to_optimize_for_budget_multiple_edge(void)
{
    cifre_conf_t config;
    set_config(CONFIG_REDUCED_FILE_PATH,&config);
    config.budget =  1.3;
    long double budget_left;
    long double expexted_cost_saved[2] = {0.1, 1.1};
    long double expexted_id[2] = {6, 2};
    int taille = 0;
    selected_edge_t *selected_edges;

    get_edges_to_optimize_for_budget(&config,&budget_left, &selected_edges);

    selected_edge_t *temp = selected_edges;
    while (temp != NULL)
    {
        TEST_ASSERT_EQUAL_MESSAGE(expexted_id[taille], temp->edge_id, "Only one edge must be selected for the budget");
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expexted_cost_saved[taille], temp->cost_saved, "Only one edge must be selected for the budget");
        temp = temp->next;
        taille++;
    }
    TEST_ASSERT_EQUAL_MESSAGE(2, taille, "Only two edge must be selected for the budget");
    free_select_edges(selected_edges);
}

/***
 * path 1 :
 *      vertex in visibility : 1,2,3,4
 * path 2 :
 *      vertex in visibiity : 1,4,6,7
 *
 *
 *
 * ***/
void test_get_edges_to_optimize_for_budget_one_edge(void)
{
    cifre_conf_t config;
    set_config(CONFIG_REDUCED_FILE_PATH,&config);
    config.budget =  0.5;
    long double buget_left;
    int taille = 0;
    selected_edge_t *selected_edges = NULL;

    get_edges_to_optimize_for_budget(&config,&buget_left, &selected_edges);

    selected_edge_t *temp = selected_edges;
    while (temp != NULL)
    {
        temp = temp->next;
        taille++;
    }
    TEST_ASSERT_EQUAL_MESSAGE(1, taille, "");
    TEST_ASSERT_EQUAL_MESSAGE(2, selected_edges->edge_id, "The id ");
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(1.1, selected_edges->cost_saved, "Only one edge must be selected for the budget");
    free_select_edges(selected_edges);
}


void test_vertices_out_array_scaling(void)
{
    int ret_code;
    vertex_t vertices[DEFAULT_NEIGHBOURS + 2];
    edge_t **edges = calloc((DEFAULT_NEIGHBOURS + 2), sizeof(edge_t *));

    for (unsigned int i = 0; i < DEFAULT_NEIGHBOURS + 2; i++)
    {
        vertices[i].nb_edges_in = 0;
        vertices[i].max_edges_in = DEFAULT_NEIGHBOURS;
        vertices[i].in_edges = calloc(DEFAULT_NEIGHBOURS, sizeof(edge_t *));
        vertices[i].nb_edges_out = 0;
        vertices[i].max_edges_out = DEFAULT_NEIGHBOURS;
        vertices[i].out_edges = calloc(DEFAULT_NEIGHBOURS, sizeof(edge_t *));
        vertices[i].id = i;

        // first vertex is the predecessor vertex for all edges
        if (i == 0)
        {
            continue;
        }
        ret_code = create_edge(i, &edges[i], 1, 1, &vertices[0], &vertices[i]);
        if (ret_code != OK)
        {
            return;
        }
    }
    unsigned int max_id = 0;
    unsigned int current_id;
    for (unsigned int i = 0; i < vertices[0].nb_edges_out; i++)
    {
        current_id = vertices[0].out_edges[i]->succ->id;
        max_id = current_id > max_id ? current_id : max_id;
    }
    TEST_ASSERT_EQUAL_MESSAGE(DEFAULT_NEIGHBOURS + 1, max_id, "probleme");
    TEST_ASSERT_EQUAL_MESSAGE(DEFAULT_NEIGHBOURS * 2, vertices[0].max_edges_out, "probleme");
    for (int i = 0; i < DEFAULT_NEIGHBOURS + 2; i++)
    {
        free(vertices[i].in_edges);
        free(vertices[i].out_edges);
        free(edges[i]);
    }
    free(edges);
}

void test_vertices_in_array_scaling(void)
{
    int ret_code;
    vertex_t vertices[DEFAULT_NEIGHBOURS + 2];
    edge_t **edges = calloc(1, sizeof(edge_t *) * (DEFAULT_NEIGHBOURS + 2));
    for (unsigned int i = 0; i < DEFAULT_NEIGHBOURS + 2; i++)
    {

        vertices[i].nb_edges_in = 0;
        vertices[i].max_edges_in = DEFAULT_NEIGHBOURS;
        vertices[i].in_edges = calloc(DEFAULT_NEIGHBOURS, sizeof(edge_t *));
        vertices[i].nb_edges_out = 0;
        vertices[i].max_edges_out = DEFAULT_NEIGHBOURS;
        vertices[i].out_edges = calloc(DEFAULT_NEIGHBOURS, sizeof(edge_t *));
        vertices[i].id = i;

        // first vertex is the successor vertex for all edges
        if (i == 0)
        {
            continue;
        }
        ret_code = create_edge(i, &edges[i], 1, 1, &vertices[i], &vertices[0]);
        if (ret_code != OK)
        {
            return;
        }
    }
    unsigned int max_id = 0;
    unsigned int current_id;
    for (unsigned int i = 0; i < vertices[0].nb_edges_in; i++)
    {
        current_id = vertices[0].in_edges[i]->pred->id;
        max_id = current_id > max_id ? current_id : max_id;
    }
    TEST_ASSERT_EQUAL_MESSAGE(DEFAULT_NEIGHBOURS + 1, max_id, "probleme");
    TEST_ASSERT_EQUAL_MESSAGE(DEFAULT_NEIGHBOURS * 2, vertices[0].max_edges_in, "probleme");
    for (int i = 0; i < DEFAULT_NEIGHBOURS + 2; i++)
    {
        free(vertices[i].in_edges);
        free(vertices[i].out_edges);
        free(edges[i]);
    }
    free(edges);
}

void test_djikstra_forward_vs_backward_path(void)
{
    vertex_t **graph;
    edge_t **edge_array;
    path_t **paths;
    cifre_conf_t config;
    set_config(CONFIG_FILE_PATH,&config);
    uint32_t nb_vertices, nb_paths, nb_edges;
    int ret_code;
    double cost;

    double *dist_array_forward, *dist_array_backward;
    int *parent_array_forward, *parent_array_backward;
    int current,old_current;

    get_graph(&config, &graph, &edge_array, &nb_vertices, &nb_edges);
    ret_code = get_paths(&config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        return;
    }

    dist_array_forward = calloc(nb_vertices, sizeof(double));
    dist_array_backward = calloc(nb_vertices, sizeof(double));

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        cost = djikstra_forward(graph, nb_vertices, &dist_array_forward, &parent_array_forward, paths[i]);
        // dijistkra_test(dist_array_forward, &dist_array_backward,parent_array_forward,&parent_array_backward,paths[i]->destination, djikstra_cost, nb_vertices);
        djikstra_backward(graph, nb_vertices, &dist_array_backward, &parent_array_backward, paths[i]);
        // used to get the number of element
        // when current=-1 there is no parent and the previous current is equal to the
        // destination for backward djikstra
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
                TEST_ASSERT_EQUAL_MESSAGE(cost,dist_array_forward[old_current],"the distance to the destionation should be the same than the djikstra cost");
            }
        }
        free(parent_array_backward);
        free(parent_array_forward);
    }
    free(dist_array_forward);
    free(dist_array_backward);
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_paths(paths, nb_paths);
}

void test_djikstra_forward_vs_backward_cost(void)
{
    vertex_t **graph;
    edge_t **edge_array;
    path_t **paths;
    cifre_conf_t config;
    set_config(CONFIG_FILE_PATH,&config);
    int ret_code;
    uint32_t nb_vertices, nb_edges, nb_paths;
    double *dist_array_forward, *dist_array_backward;
    // int *parent_f;
    double cost_forward, cost_backward;
    // char str[80];

    get_graph(&config, &graph, &edge_array, &nb_vertices, &nb_edges);
    ret_code = get_paths(&config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        return;
    }

    dist_array_forward = calloc(nb_vertices, sizeof(double));
    dist_array_backward = calloc(nb_vertices, sizeof(double));

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        cost_forward = djikstra_forward(graph, nb_vertices, &dist_array_forward, NULL, paths[i]);
        cost_backward = djikstra_backward(graph, nb_vertices, &dist_array_backward, NULL, paths[i]);
        // cost_forward = djikstra_forward(graph, nb_vertices, &dist_array_forward, &parent_f, &paths[i]);
        // cost_backward = dijistkra_backward_2(nb_vertices,dist_array_forward, &dist_array_backward,parent_f, cost_forward,&paths[i]);

        // sprintf(str, "path %d failed, incorrect cps calculated", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.001, cost_forward, cost_backward, "salut");

        // free(parent_f);
    }
    free(dist_array_forward);
    free(dist_array_backward);
    free_edge(edge_array, nb_edges);
    free_graph(graph, nb_vertices);
    free_paths(paths, nb_paths);
}

void test_djikstra_forward(void)
{

    vertex_t **graph;
    edge_t **edge_array;
    path_t **paths;
    cifre_conf_t config;
    set_config(CONFIG_FILE_PATH,&config);
    int ret_code;
    uint32_t nb_vertices, nb_edges, nb_paths;
    double *dist_array;
    double cost, expected_cost;
    // char str[80];

    get_graph(&config, &graph, &edge_array, &nb_vertices, &nb_edges);
    ret_code = get_paths(&config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        return;
    }

    dist_array = calloc(nb_vertices, sizeof(double));
    for (uint32_t i = 0; i < nb_paths; i++)
    {
        cost = djikstra_forward(graph, nb_vertices, &dist_array, NULL, paths[i]);
        expected_cost = calc_path_cps(paths[i]->cps_djikstra_dist, paths[i]->cps_djikstra_danger, paths[i]->profil);

        // sprintf(str, "path %d failed, incorrect cps calculated", i);
        TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(0.000001, expected_cost, cost, "str");
    }
    free(dist_array);
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
    double return_value = cost_function(&path.profil, &edge.dist, &edge.danger);
    TEST_ASSERT_EQUAL_DOUBLE(edge.dist, return_value);

    // when the alpha is equal 0.5, the cost function should return the
    //  edge distance + the edge danger divided by two
    path.profil = 0.5;
    return_value = cost_function(&path.profil, &edge.dist, &edge.danger);

    // when the alpha is equal 1, the cost function should return the
    //  edge danger
    TEST_ASSERT_EQUAL_DOUBLE(3.5, return_value);
    path.profil = 0;
    return_value = cost_function(&path.profil, &edge.dist, &edge.danger);
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
    RUN_TEST(test_get_edges_to_optimize_for_budget_thread_vs_single);
    RUN_TEST(test_get_edges_to_optimize_for_budget_one_edge);
    RUN_TEST(test_get_edges_to_optimize_for_budget_no_edge);
    RUN_TEST(test_get_edges_to_optimize_for_budget_multiple_edge);
    return UNITY_END();
}