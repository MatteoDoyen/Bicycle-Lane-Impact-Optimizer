#include "array_scaling_test.h"
#include <stdio.h>
#include <stdlib.h>
#include "unity.h"
#include "cost_function_test.h"
#include "../header/graph.h"
#include "../header/path.h"
#include "util.h"

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