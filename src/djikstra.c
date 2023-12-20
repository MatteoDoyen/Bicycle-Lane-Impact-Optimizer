#include "../header/djikstra.h"
#include <stdbool.h>

void init_cost_diff_array(cost_diff_arc_t *diff_array, unsigned int nbArc)
{
    for (unsigned int i = 0; i < nbArc; i++)
    {
        diff_array[i].djikstra_cost_diff = 0;
    }
}

void get_max_arc_to_optimize(cost_diff_arc_t *diff_array, unsigned int nbArc, int *arc_id_to_optimize, long double *budget_left)
{
    double max_cost_saved = 0;
    unsigned int max_cost_arc_id;
    // cost_diff_arc_t *temp;

    for (unsigned int id_arc = 0; id_arc < nbArc; id_arc++)
    {

        if (diff_array[id_arc].dist <= (*budget_left) && diff_array[id_arc].djikstra_cost_diff > max_cost_saved)
        {
            max_cost_saved = diff_array[id_arc].djikstra_cost_diff;
            max_cost_arc_id = id_arc;
        }
    }
    if (max_cost_saved > 0)
    {
        *arc_id_to_optimize = max_cost_arc_id;
        printf("%d %f", max_cost_arc_id, max_cost_saved);
    }
}

double cost_function(trace_t *trace, Arc *arc)
{
    return (trace->profil * arc->dist) + ((1 - trace->profil) * arc->danger);
    // return arc->dist;
}

/// update the distance in the cached djikstra
/// here the vertex already has an updated distance
double updated_dist(Arc *arc, trace_t *trace)
{
    return trace->foward_djikstra[arc->predecesseur->id] + cost_function(trace, arc) + trace->backward_djikstra[arc->succ->id];
}

double djikstra_forward(struct Noeud *graph, int V, double **dist_array,int** parent_array, trace_t *trace)
{
    (*dist_array) = (double *)calloc(1,V * sizeof(double));
    int * parent;
    bool markedVertex[V];
    bool toVisitVertex[V];
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;

    if(parent_array!=NULL){
        (*parent_array) = (int *)calloc(1,V * sizeof(int));
        parent = (*parent_array);
    }else{
        parent = (int *)calloc(1,V * sizeof(int));
    }

    origin = trace->origin;
    destination = trace->destination;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < V; i++)
    {
        (*dist_array)[i] = DBL_MAX;
        markedVertex[i] = false;
        toVisitVertex[i] = false;
    }

    // Distance from source to itself is always 0
    (*dist_array)[origin] = 0;

    parent[origin] = -1;

    verticesToVisit = calloc(1,sizeof(list_node_t));
    verticesToVisit->vertex_id = origin;
    verticesToVisit->next = NULL;
    for (unsigned int i = 0; i < graph[origin].nb_arc_sortant; i++)
    {
        dest_vertex_id = graph[origin].sortant[i]->succ->id;

        list_node_t *newNode = calloc(1,sizeof(list_node_t));
        toVisitVertex[dest_vertex_id] = true;
        newNode->vertex_id = dest_vertex_id;
        newNode->next = verticesToVisit;
        verticesToVisit = newNode;
    }

    // Find the shortest path for all vertices
    while (verticesToVisit != NULL)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        int u = minDistance((*dist_array), verticesToVisit);
        if (u == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[u] = true;

        temp = verticesToVisit;
        old = NULL;
        while (temp != NULL && temp->vertex_id != u)
        {
            old = temp;
            temp = temp->next;
        }
        if (old != NULL)
        {
            old->next = temp->next;
        }
        else
        {
            verticesToVisit = temp->next;
        }
        free(temp);

        // Update dist value of the adjacent vertices
        for (unsigned int arc_i = 0; arc_i < graph[u].nb_arc_sortant; arc_i++)
        {
            dest_vertex_id = graph[u].sortant[arc_i]->succ->id;
            if (!vertexIsInVisiblite(trace, dest_vertex_id))
            {
                continue;
            }
            if (!markedVertex[dest_vertex_id] && graph[u].sortant[arc_i] != NULL && ((*dist_array)[u] + cost_function(trace, graph[u].sortant[arc_i]) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(trace, graph[u].sortant[arc_i]);
                parent[dest_vertex_id] = u;
                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    list_node_t *vertexToVisit = calloc(1,sizeof(list_node_t));
                    vertexToVisit->next = verticesToVisit;
                    vertexToVisit->vertex_id = dest_vertex_id;
                    verticesToVisit = vertexToVisit;
                }
            }
        }
    }
    if(parent_array==NULL){
        free(parent);
    }
    return (*dist_array)[destination];
}

double djikstra_backward(struct Noeud *graph, int V, double **dist_array,int **parent_array, trace_t *trace)
{
    (*dist_array) = (double *)calloc(1,V * sizeof(double));
    bool markedVertex[V];
    bool toVisitVertex[V];
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;
    int *parent;

    if(parent_array!=NULL){
        (*parent_array) = (int *)calloc(1,V * sizeof(int));
        parent = (*parent_array);
    }else{
        parent = (int *)calloc(1,V * sizeof(int));
    }

    origin = trace->destination;
    destination = trace->origin;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < V; i++)
    {
        (*dist_array)[i] = DBL_MAX;
        markedVertex[i] = false;
        toVisitVertex[i] = false;
    }

    // Distance from source to itself is always 0
    (*dist_array)[origin] = 0;

    parent[origin] = -1;


    verticesToVisit = calloc(1,sizeof(list_node_t));
    verticesToVisit->vertex_id = origin;
    verticesToVisit->next = NULL;
    for (unsigned int i = 0; i < graph[origin].nb_arc_entrant; i++)
    {
        dest_vertex_id = graph[origin].entrant[i]->predecesseur->id;

        list_node_t *newNode = calloc(1,sizeof(list_node_t));
        toVisitVertex[dest_vertex_id] = true;
        newNode->vertex_id = dest_vertex_id;
        newNode->next = verticesToVisit;
        verticesToVisit = newNode;
    }

    // Find the shortest path for all vertices
    while (verticesToVisit != NULL)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        int u = minDistance((*dist_array), verticesToVisit);
        if (u == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[u] = true;

        temp = verticesToVisit;
        old = NULL;
        while (temp != NULL && temp->vertex_id != u)
        {
            old = temp;
            temp = temp->next;
        }
        if (old != NULL)
        {
            old->next = temp->next;
        }
        else
        {
            verticesToVisit = temp->next;
        }
        free(temp);

        // Update dist value of the adjacent vertices
        for (unsigned int arc_i = 0; arc_i < graph[u].nb_arc_entrant; arc_i++)
        {
            dest_vertex_id = graph[u].entrant[arc_i]->predecesseur->id;
            if (!vertexIsInVisiblite(trace, dest_vertex_id))
            {
                continue;
            }
            if (!markedVertex[dest_vertex_id] && graph[u].entrant[arc_i] != NULL && ((*dist_array)[u] + cost_function(trace, graph[u].entrant[arc_i]) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(trace, graph[u].entrant[arc_i]);
                parent[dest_vertex_id] = u;
                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    list_node_t *vertexToVisit = calloc(1,sizeof(list_node_t));
                    vertexToVisit->next = verticesToVisit;
                    vertexToVisit->vertex_id = dest_vertex_id;
                    verticesToVisit = vertexToVisit;
                }
            }
        }
    }
    if(parent_array==NULL){
        free(parent);
    }

    return (*dist_array)[destination];
}

int minDistance(double dist[], list_node_t *vertexToVisit)
{
    double min = DBL_MAX;
    int min_index = -1;

    list_node_t *temp = vertexToVisit;

    while (temp != NULL)
    {
        if (dist[temp->vertex_id] < min)
        {
            min = dist[temp->vertex_id];
            min_index = temp->vertex_id;
        }
        temp = temp->next;
    }
    return min_index;
}
// TODO : REMOVE PARENTS
//  Dijkstra's algorithm to find the shortest path between two vertices
double djikstra_test(struct Noeud *graph, int V, double **dist_array, int **parent, trace_t *trace)
{
    (*dist_array) = (double *)calloc(1,V * sizeof(double));
    bool markedVertex[V];
    bool toVisitVertex[V];
    (*parent) = (int *)calloc(1,V * sizeof(int));
    int dest_vertex_id;
    unsigned int origin, destination;
    list_node_t *verticesToVisit = NULL;
    list_node_t *temp, *old;

    origin = trace->origin;
    destination = trace->destination;

    // Initialize distances, set all vertices as not yet included in the shortest path tree
    for (int i = 0; i < V; i++)
    {
        (*dist_array)[i] = DBL_MAX;
        markedVertex[i] = false;
        toVisitVertex[i] = false;
    }

    // Distance from source to itself is always 0
    (*dist_array)[origin] = 0;

    // Parent of source is itself
    (*parent)[origin] = -1;

    verticesToVisit = calloc(1,sizeof(list_node_t));
    verticesToVisit->vertex_id = origin;
    verticesToVisit->next = NULL;
    for (unsigned int i = 0; i < graph[origin].nb_arc_sortant; i++)
    {
        dest_vertex_id = graph[origin].sortant[i]->succ->id;

        list_node_t *newNode = calloc(1,sizeof(list_node_t));
        toVisitVertex[dest_vertex_id] = true;
        newNode->vertex_id = dest_vertex_id;
        newNode->next = verticesToVisit;
        verticesToVisit = newNode;
    }

    // Find the shortest path for all vertices
    while (verticesToVisit != NULL)
    {
        // Pick the minimum distance vertex from the set of vertices not yet processed
        int u = minDistance((*dist_array), verticesToVisit);
        if (u == -1)
        {
            break;
        }
        // Mark the picked vertex as processed
        markedVertex[u] = true;

        temp = verticesToVisit;
        old = NULL;
        while (temp != NULL && temp->vertex_id != u)
        {
            old = temp;
            temp = temp->next;
        }
        if (old != NULL)
        {
            old->next = temp->next;
        }
        else
        {
            verticesToVisit = temp->next;
        }
        free(temp);

        // Update dist value of the adjacent vertices
        for (unsigned int arc_i = 0; arc_i < graph[u].nb_arc_sortant; arc_i++)
        {
            dest_vertex_id = graph[u].sortant[arc_i]->succ->id;
            if (!vertexIsInVisiblite(trace, dest_vertex_id))
            {
                continue;
            }
            if (!markedVertex[dest_vertex_id] && graph[u].sortant[arc_i] != NULL && ((*dist_array)[u] + cost_function(trace, graph[u].sortant[arc_i]) < (*dist_array)[dest_vertex_id]))
            {
                (*dist_array)[dest_vertex_id] = (*dist_array)[u] + cost_function(trace, graph[u].sortant[arc_i]);
                (*parent)[dest_vertex_id] = u;

                if (!toVisitVertex[dest_vertex_id])
                {
                    toVisitVertex[dest_vertex_id] = true;
                    list_node_t *vertexToVisit = calloc(1,sizeof(list_node_t));
                    vertexToVisit->next = verticesToVisit;
                    vertexToVisit->vertex_id = dest_vertex_id;
                    verticesToVisit = vertexToVisit;
                }
            }
        }
    }

    return (*dist_array)[destination];
}