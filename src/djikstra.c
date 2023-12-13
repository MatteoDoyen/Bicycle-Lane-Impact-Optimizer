#include "../header/djikstra.h"

void init_cost_diff_array(cost_diff_arc_t *diff_array, unsigned int nbArc)
{
    for (unsigned int i = 0; i < nbArc; i++)
    {
        diff_array[i].suivant = NULL;
    }
}

void new_cost_diff(unsigned int id_trace, double djikstra_cost_diff, cost_diff_arc_t *arc_p)
{

    cost_diff_arc_t *temp_diff_array = arc_p;
    while (temp_diff_array->suivant != NULL)
    {
        temp_diff_array = temp_diff_array->suivant;
    }
    temp_diff_array->id_trace = id_trace;
    temp_diff_array->djikstra_cost_diff = djikstra_cost_diff;
    temp_diff_array->suivant = malloc(sizeof(cost_diff_arc_t));
    temp_diff_array->suivant->suivant = NULL;
}

void get_max_arc_to_optimize(cost_diff_arc_t *diff_array, unsigned int nbArc, unsigned int * arc_id_to_optimize, long double* budget_left)
{
    double max_cost_saved  = 0;
    double local_max_cost_saved;
    unsigned int max_cost_arc_id;
    cost_diff_arc_t * temp;

    for (unsigned int id_arc = 0; id_arc < nbArc; id_arc++)
    {
        local_max_cost_saved = 0;
        temp = &diff_array[id_arc];
        while (temp->suivant != NULL)
        {
            local_max_cost_saved = diff_array[id_arc].djikstra_cost_diff > local_max_cost_saved ? diff_array[id_arc].djikstra_cost_diff : local_max_cost_saved;
            temp = temp->suivant;
        }
        if(local_max_cost_saved>max_cost_saved && local_max_cost_saved<=(*budget_left)){
            max_cost_saved = local_max_cost_saved;
            max_cost_arc_id = id_arc;
        }
    }
    if(max_cost_saved>0){
        *arc_id_to_optimize = max_cost_arc_id;
    }
    
}

void print_cost_diff(cost_diff_arc_t *diff_array, unsigned int nbArc)
{

    double max_cost_saved  = 0;
    unsigned int max_cost_trace_id;
    cost_diff_arc_t * temp;

    for (unsigned int id_arc = 0; id_arc < nbArc; id_arc++)
    {
        max_cost_saved = 0;
        temp = &diff_array[id_arc];
        while (temp->suivant != NULL)
        {
            max_cost_saved = diff_array[id_arc].djikstra_cost_diff > max_cost_saved ? diff_array[id_arc].djikstra_cost_diff : max_cost_saved;
            max_cost_trace_id  = temp->id_trace;
            temp = temp->suivant;
        }
        if(max_cost_saved>0){
            printf("arc %d max cost saved %f id_trace : %d\n",id_arc,max_cost_saved,max_cost_trace_id);
        }
    }
}

double cost_function(trace_t *trace, Arc *arc)
{
    return (trace->profil * arc->dist) + ((1 - trace->profil) * arc->danger);
}

/// update the distance in the cached djikstra
/// here the vertex already has an updated distance
double updated_dist(Arc *arc, trace_t *trace)
{
    return trace->foward_djikstra[arc->predecesseur->id] + cost_function(trace, arc) + trace->backward_djikstra[arc->succ->id];
}
// Function to find the vertex with the minimum distance value
int minDistance(double dist[], bool markedVertex[], int V)
{
    double min = DBL_MAX;
    int min_index = -1;

    for (int v = 0; v < V; v++)
    {
        if ((markedVertex[v] == false) && dist[v] < min)
        {
            min = dist[v];
            min_index = v;
        }
    }
    return min_index;
}

// Dijkstra's algorithm to find the shortest path between two vertices
double djikstra(struct Noeud *graph, int V, double **dist_array, trace_t *trace)
{
    (*dist_array) = (double *)malloc(V * sizeof(double));
    bool *markedVertex = (bool *)malloc(V * sizeof(bool));
    int *parent = (int *)malloc(V * sizeof(int));
    int dest_vertex_id;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < V; i++)
    {
        (*dist_array)[i] = DBL_MAX;
        markedVertex[i] = false;
    }

    // Distance from source to itself is always 0
    (*dist_array)[trace->origin] = 0;

    // Parent of source is itself
    parent[trace->origin] = -1;

    // Find the shortest path for all vertices
    for (int count = 0; count < V - 1; count++)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        int u = minDistance((*dist_array), markedVertex, V);
        if (u == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[u] = true;
        // Update dist value of the adjacent vertices
        for (unsigned int arc_i = 0; arc_i < graph[u].nb_arc_sortant; arc_i++)
        {
            dest_vertex_id = graph[u].sortant[arc_i]->succ->id;
            // if (!markedVertex[dest_vertex_id] && graph[u].sortant[arc_i] != NULL && ((*dist_array)[u] + graph[u].sortant[arc_i]->dist < (*dist_array)[dest_vertex_id]))
            if (!markedVertex[dest_vertex_id] && graph[u].sortant[arc_i] != NULL && ((*dist_array)[u] + cost_function(trace, graph[u].sortant[arc_i]) < (*dist_array)[dest_vertex_id]))
            {
                // (*dist_array)[dest_vertex_id] = (*dist_array)[u] + graph[u].sortant[arc_i]->dist;
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(trace, graph[u].sortant[arc_i]);
                parent[dest_vertex_id] = u;
            }
        }
    }

    // Print the shortest path from source to destination
    // printf("Shortest Path from %d to %d:\n", trace->origin, trace->destination);
    // int current = trace->destination;
    // while (current != -1)
    // {
    //     printf("%d <- ", current);
    //     current = parent[current];
    // }
    // printf("\n");

    // // // Print the total distance of the shortest path
    // printf("Total Distance: %f\n", (*dist_array)[trace->destination]);

    free(markedVertex);
    free(parent);
    return (*dist_array)[trace->destination];
}