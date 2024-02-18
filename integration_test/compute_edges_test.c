#include "compute_edges_test.h"
#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "config.h"
#include "compute_edges.h"
#include "../header/dijkstra.h"
#include "util.h"

#define CONFIG_FILE_PATH "./conf_test/conf_unit_test.json"
#define CONFIG_REDUCED_FILE_PATH "./conf_test/conf_unit_test_reduced.json"
#define CONFIG_REAL_FILE_PATH "./conf_test/conf_unit_test_real.json"
#define PRECALCULATED_COMPUTE_EDGES "./data_test/selected_edges_dummy.csv"

void test_get_edges_to_optimize_for_budget_no_edge(void)
{
    config_t config;
    get_config(CONFIG_REDUCED_FILE_PATH,&config);
    config.budget = 0.49;
    long double budget_used; // lowest dist to optimize is 0.5
    int taille = 0;
    double_unsigned_list_t *selected_edges;

    get_edges_to_optimize_for_budget(&config,&budget_used,&selected_edges);

    double_unsigned_list_t *temp = selected_edges;
    while (temp != NULL)
    {
        temp = temp->next;
        taille++;
    }
    TEST_ASSERT_EQUAL_MESSAGE(0, taille, "Only two edge must be selected for the budget");
    free_double_unsigned_list_t(selected_edges);
    free_config(&config);
}

void test_get_edges_to_optimize_for_budget_multiple_edges(void)
{
    config_t config;
    get_config(CONFIG_REDUCED_FILE_PATH,&config);
    config.budget =  1.3;
    long double budget_used;
    long double expexted_cost_saved[2] = {0.1, 1.1};
    long double expexted_id[2] = {6, 2};
    int taille = 0;
    double_unsigned_list_t *selected_edges;

    get_edges_to_optimize_for_budget(&config,&budget_used, &selected_edges);

    double_unsigned_list_t *temp = selected_edges;
    while (temp != NULL)
    {
        TEST_ASSERT_EQUAL_MESSAGE(expexted_id[taille], temp->u_value, "Only one edge must be selected for the budget");
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expexted_cost_saved[taille], temp->d_value, "Only one edge must be selected for the budget");
        temp = temp->next;
        taille++;
    }
    TEST_ASSERT_EQUAL_MESSAGE(2, taille, "Only two edge must be selected for the budget");
    free_double_unsigned_list_t(selected_edges);
    free_config(&config);
}

void test_get_edges_to_optimize_for_budget_one_edge(void)
{
    config_t config;
    get_config(CONFIG_REDUCED_FILE_PATH,&config);
    config.budget =  0.5;
    long double buget_left;
    int taille = 0;
    double_unsigned_list_t *selected_edges = NULL;

    get_edges_to_optimize_for_budget(&config,&buget_left, &selected_edges);

    double_unsigned_list_t *temp = selected_edges;
    while (temp != NULL)
    {
        temp = temp->next;
        taille++;
    }
    TEST_ASSERT_EQUAL_MESSAGE(1, taille, "");
    TEST_ASSERT_EQUAL_MESSAGE(2, selected_edges->u_value, "The id ");
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(1.1, selected_edges->d_value, "Only one edge must be selected for the budget");
    free_double_unsigned_list_t(selected_edges);
    free_config(&config);
}

void test_get_edges_to_optimize_for_budget_no_budget(void){
    config_t config;
    get_config(CONFIG_REDUCED_FILE_PATH,&config);
    config.budget =  0;
    long double budget_used;
    long double expexted_cost_saved[2] = {0.1, 1.1};
    long double expexted_id[2] = {6, 2};
    int taille = 0;
    double_unsigned_list_t *selected_edges;

    get_edges_to_optimize_for_budget(&config,&budget_used, &selected_edges);

    double_unsigned_list_t *temp = selected_edges;
    while (temp != NULL)
    {
        TEST_ASSERT_EQUAL_MESSAGE(expexted_id[taille], temp->u_value, "Only one edge must be selected for the budget");
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expexted_cost_saved[taille], temp->d_value, "Only one edge must be selected for the budget");
        temp = temp->next;
        taille++;
    }
    TEST_ASSERT_EQUAL_MESSAGE(0, taille, "Only two edge must be selected for the budget");
    free_double_unsigned_list_t(selected_edges);
    free_config(&config);
}

void test_get_edges_to_optimize_for_budget_budget_over_max(void){
    config_t config;
    get_config(CONFIG_REDUCED_FILE_PATH,&config);
    config.budget =  5;
    long double budget_used;
    long double expexted_cost_saved[2] = {0.1, 1.1};
    long double expexted_id[2] = {6, 2};
    int taille = 0;
    double_unsigned_list_t *selected_edges;

    get_edges_to_optimize_for_budget(&config,&budget_used, &selected_edges);

    double_unsigned_list_t *temp = selected_edges;
    while (temp != NULL)
    {
        TEST_ASSERT_EQUAL_MESSAGE(expexted_id[taille], temp->u_value, "Only one edge must be selected for the budget");
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(expexted_cost_saved[taille], temp->d_value, "Only one edge must be selected for the budget");
        temp = temp->next;
        taille++;
    }
    TEST_ASSERT_EQUAL_MESSAGE(2, taille, "Only two edge must be selected for the budget");
    free_double_unsigned_list_t(selected_edges);
    free_config(&config);
}

void test_get_edges_to_optimize_for_budget_vs_precomputed(void){
    config_t config;
    get_config(CONFIG_FILE_PATH,&config);
    
    int taille = 0;
    double_unsigned_list_t *selected_edges;
    long double budget_used;
    char *** csv_matrix;
    uint32_t current_edge_id;
    double current_edge_value;
    uint32_t nb_rows,nb_cols;

    get_edges_to_optimize_for_budget(&config,&budget_used, &selected_edges);
    int ret = read_csv_file(PRECALCULATED_COMPUTE_EDGES,&csv_matrix,&nb_rows,&nb_cols,";");
    TEST_ASSERT_EQUAL_MESSAGE(OK, ret, "Error while reading the csv file");

    double_unsigned_list_t *temp = selected_edges;
    while (temp != NULL)
    {
        current_edge_id = (uint32_t)atoi(csv_matrix[taille+1][0]);
        current_edge_value = (double)atof(csv_matrix[taille+1][1]);
        TEST_ASSERT_EQUAL_MESSAGE(current_edge_id, temp->u_value, "wrong id for the edge");
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(current_edge_value, temp->d_value, "wrong cost saved for the edge");
        temp = temp->next;
        taille++;
    }
    TEST_ASSERT_EQUAL_MESSAGE(nb_rows-1, taille, "Only two edge must be selected for the budget");
    free_double_unsigned_list_t(selected_edges);
    free_config(&config);
}