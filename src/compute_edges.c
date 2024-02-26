#include <stdbool.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "../header/util.h"
#include "../header/compute_edges.h"
#include <omp.h>

/*
 * Function: get_edges_to_optimize_for_budget
 * ------------------------------------------
 * Get all the edges that, if improved, would have the biggest impact until
 * the budget is reached.
 *
 * Parameters:
 * - config: a pointer to the configuration structure
 * - budget_used: a pointer to a variable that will store the budget used
 * - selected_edges: a pointer to a list that will store the selected edges
 *
 * Returns:
 * - an integer representing the return code
 *
 * Note:
 * - This function improves an edge, recalculates the paths that have the edge within their visibility,
 *   and compares the new Dijkstra "cost" to the old cost of the paths.
 * - If the cost is smaller, it is added to the total costs saved by improving the edge.
 * - If the best edge improvement of the graph had a non-null impact, the graph is then updated to take in this improvement.
 * - The function stops when the budget is reached or when no other edge improvements have an impact.
 *
 * Example usage:
 * ```
 * config_t config;
 * long double budget_used;
 * double_unsigned_list_t *selected_edges;
 * int ret_code = get_edges_to_optimize_for_budget(&config, &budget_used, &selected_edges);
 * if (ret_code == OK) {
 *     // Process the selected edges and the budget used
 * } else {
 *     // Handle the error
 * }
 * ```
 */
int get_edges_to_optimize_for_budget(config_t *config, long double *budget_used, double_unsigned_list_t **selected_edges)
{
    graph_t graph;
    path_t **paths;
    unsigned_list_t **path_list =NULL;
    unsigned_list_t *current_path;
    int ret_code;
    (*selected_edges) = NULL;
    long double max_saved_cost;
    uint32_t nb_paths;
    long double budget_left = config->budget;
    int32_t edge_id_to_optimize;
    double *dijkstra_backward_dist, *dijkstra_forward_dist;
    long double new_dijkstra_cost, dijkstra_cost, cost_difference;
    bool stop = false;
    uint32_t t_it;
    uint32_t path_id;
    
    ret_code = get_graph(config, &graph);
    if (ret_code != OK)
    {
        return ret_code;
    }
    ret_code = get_paths(config, &paths, &nb_paths);
    if (ret_code != OK)
    {
        free_graph(&graph);
        return ret_code;
    }

    bool *impact = calloc(nb_paths, sizeof(bool));
    if (impact == NULL)
    {
        free_graph(&graph);
        free_paths(paths, nb_paths);
        fprintf(stderr, "Memory allocation failed for impact array\n");
        return MEMORY_ALLOC_ERROR;
    }

    // Initialize the impact array to true so that all the paths are considered
    for (uint32_t i = 0; i < nb_paths; i++)
    {
        impact[i] = true;
    }

    // This array is used to get the cost saved by improving an edge
    // for a given path
    double_unsigned_list_t **cost_diff_array;
    ret_code = init_cost_diff_array(&cost_diff_array, nb_paths);
    if (ret_code != OK)
    {
        free_graph(&graph);
        free_paths(paths, nb_paths);
        free(impact);
        fprintf(stderr, "Memory allocation failed for cost_diff_array array\n");
        return MEMORY_ALLOC_ERROR;
    }

    omp_set_num_threads(config->thread_number);

        while (!stop)
    {
        // this function assigns the traces to the threads
        assign_traces_to_threads(config, paths, nb_paths, &path_list, impact);
#pragma omp parallel shared(path_list, nb_paths, impact, graph, paths, cost_diff_array) private(path_id, t_it, current_path, dijkstra_forward_dist, dijkstra_backward_dist, dijkstra_cost, new_dijkstra_cost, cost_difference)
        {
            dijkstra_forward_dist = calloc(graph.nb_vertices, sizeof(double));
            dijkstra_backward_dist = calloc(graph.nb_vertices, sizeof(double));

            t_it = omp_get_thread_num();
            current_path = path_list[t_it];

            while (current_path != NULL)
            {
                path_id = current_path->u_value;
                impact[path_id] = false;

                //  Calculating the dijkstra path backward and forward allows for
                //  a much faster computing of the impact of the improvement of an edge
                dijkstra_cost = dijkstra_backward(&graph, &dijkstra_backward_dist, NULL, paths[path_id]);
                dijkstra_forward(&graph, &dijkstra_forward_dist, NULL, paths[path_id]);

                for (uint32_t edge_id = 0; edge_id < graph.nb_edges; edge_id++)
                {
                    if (graph.edge_array[edge_id]->dist > budget_left)
                    {
                        continue;
                    }
                    //  if the edge's vertexes are in the visibility of the path
                    //  and the edge is not already optimized
                    if (edge_is_in_visibilite(paths[path_id], graph.edge_array[edge_id]) && (graph.edge_array[edge_id]->dist != graph.edge_array[edge_id]->danger))
                    {

                        new_dijkstra_cost = updated_dist(graph.edge_array[edge_id], dijkstra_forward_dist, dijkstra_backward_dist);

                        if (dijkstra_cost > new_dijkstra_cost)
                        {
                            cost_difference = dijkstra_cost - new_dijkstra_cost;
#pragma omp critical
                            add_double_unsigned_list_t(&cost_diff_array[path_id], edge_id, cost_difference);
                        }
                    }
                }
                current_path = current_path->next;
            }
            free(dijkstra_forward_dist);
            free(dijkstra_backward_dist);
        }
        // init at -1 so that we can check if we found an edge to optimize
        edge_id_to_optimize = -1;
        get_max_edge_to_optimize(cost_diff_array, nb_paths, &graph, &edge_id_to_optimize, &max_saved_cost, budget_left);
        if (edge_id_to_optimize == -1)
        {
            stop = true;
        }
        else
        {
            for (uint32_t path_id = 0; path_id < nb_paths; path_id++)
            {
                if (edge_is_in_visibilite(paths[path_id], graph.edge_array[edge_id_to_optimize]))
                {
                    impact[path_id] = true;
                    free_double_unsigned_list_t(cost_diff_array[path_id]);
                    cost_diff_array[path_id] = NULL;
                }
            }
            ret_code = add_double_unsigned_list_t(selected_edges, graph.edge_array[edge_id_to_optimize]->id, max_saved_cost);
            if (ret_code != OK)
            {
                free(impact);
                free_graph(&graph);
                free_paths(paths, nb_paths);
                free_double_unsigned_list_t(*selected_edges);
                return ret_code;
            }
            budget_left = budget_left - graph.edge_array[edge_id_to_optimize]->dist;
            *budget_used = config->budget - budget_left;
            graph.edge_array[edge_id_to_optimize]->danger = graph.edge_array[edge_id_to_optimize]->dist;
}
    }
    free_path_list(config, path_list);
    free(impact);
    free_graph(&graph);
    free_paths(paths, nb_paths);
    free_cost_diff_array(cost_diff_array, nb_paths);
    return OK;
}

/**
 * Function: assign_traces_to_threads
 * ----------------------------------
 * This function is responsible for assigning traces to threads for parallel processing.
 * It ensures that the workload is evenly distributed across all available threads, 
 * optimizing the performance of the program.
 *
 * Parameters: 
 * - config: Pointer to the configuration structure containing program settings
 * - paths: Array of path structures representing the traces
 * - nb_paths: Number of paths/traces
 * - path_list_ref: Reference to the pointer of the path list for each thread
 * - impact_array: Array indicating whether a path has an impact or not
 *
 * Returns: 
 * - Integer value representing the status of the function execution
 *
 * Note: 
 * - This function assumes that the number of traces is greater than or equal to the number of threads.
 * - The function does not handle the case where a thread fails during execution.
 * - The function does not check if the threads have completed their assigned tasks before returning.
 *
 * Example usage: 
 * - assign_traces_to_threads(&config, paths, nb_paths, &path_list_ref, impact_array);
 */
int assign_traces_to_threads(config_t *config, path_t **paths, uint32_t nb_paths, unsigned_list_t ***path_list_ref, bool *impact_array) {
    uint32_t min_size = INT32_MAX;
    uint32_t min_index = -1;

    if (*path_list_ref != NULL) {
        free_path_list(config, *path_list_ref);
    }

    *path_list_ref = calloc(config->thread_number, sizeof(unsigned_list_t *));
    unsigned_list_t **path_list = *path_list_ref;
    uint32_t *paths_size = calloc(config->thread_number, sizeof(uint32_t));

    if (path_list == NULL) {
        return MEMORY_ALLOC_ERROR;
    }

    for (uint32_t i = 0; i < config->thread_number; i++) {
        paths_size[i] = 0;
        path_list[i] = NULL;
    }

    for (uint32_t path_id = 0; path_id < nb_paths; path_id++) {
        if (!impact_array[path_id]) {
            continue;
        }
        for (uint32_t j = 0; j < config->thread_number; j++) {
            if (paths_size[j] < min_size) {
                min_size = paths_size[j];
                min_index = j;
            }
        }
        // add the path_id to the list of paths to be looked at by the thread
        add_unsigned_list_t(&path_list[min_index], path_id);
        // update the current size of the paths's visibilities for the given thread
        paths_size[min_index] += paths[path_id]->nb_visibilite;
        min_size = INT32_MAX;
    }

    free(paths_size);

    return OK;
}

/**
 * Function: free_path_list
 * ------------------------
 * This function frees the memory allocated for the path list.
 *
 * Parameters:
 * - config: a pointer to the configuration structure.
 * - path_list: a double pointer to the path list.
 *
 * Returns:
 * This function does not return a value.
 *
 * Note:
 * Make sure to call this function before returning.
 *
 * Example usage:
 * ```
 * config_t config;
 * unsigned_list_t **path_list;
 * assign_traces_to_threads(&config, paths, nb_paths, &path_list_ref, impact_array);
 * free_path_list(&config, path_list);
 * ```
 */
void free_path_list(config_t *config, unsigned_list_t **path_list) {
    for (uint32_t i = 0; i < config->thread_number; i++) {
        free_unsigned_list_t(path_list[i]);
    }
    free(path_list);
}

/**
 * Function: save_selected_edges
 * -----------------------------
 * This function saves the selected edges to a file.
 *
 * Parameters:
 * - head: a pointer to the head of the double_unsigned_list_t.
 * - directory: the directory path where the file will be saved.
 * - file_path: the path of the file to be saved.
 *
 * Returns:
 * This function does not return a value.
 *
 * Note:
 * - The function creates the result directory if it does not exist.
 * - The file is opened in write mode and overwritten if it already exists.
 * - The file format is as follows:
 *   - The header contains the column names "edge_id" and "saved_cost".
 *   - Each line represents an edge with the edge ID and the saved cost separated by a semicolon.
 *
 * Example usage:
 * ```
 * double_unsigned_list_t *head = ...;
 * char *directory = "/path/to/directory";
 * char *file_path = "/path/to/directory/result.txt";
 * save_selected_edges(head, directory, file_path);
 * ```
 */
void save_selected_edges(double_unsigned_list_t *head, char *directory, char *file_path)
{
    // Create result directory if it does not exist
    mkdir(directory, 0777);

    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Error opening file: %s\n", file_path);
        exit(1);
    }

    // Write header to the file
    fprintf(file, "edge_id;saved_cost\n");

    double_unsigned_list_t *current = head;
    // Write each edge to the file
    while (current != NULL) {
        fprintf(file, "%u;%.8Lf\n", current->u_value, current->d_value);
        current = current->next;
    }

    fclose(file);
}

/**
 * Function: get_total_saved_cost
 * ------------------------------
 * This function calculates the total saved cost by summing up the values in a linked list.
 *
 * Parameters:
 * - head: A pointer to the head of the linked list.
 *
 * Returns:
 * - The total saved cost as a double.
 *
 * Note:
 * - This function assumes that the linked list is not empty.
 * - The linked list should be properly initialized and populated before calling this function.
 *
 * Example usage:
 * - double totalSavedCost = get_total_saved_cost(head);
 */
double get_total_saved_cost(double_unsigned_list_t *head){
    double sum=0;
    double_unsigned_list_t *current = head;
    while (current != NULL)
    {
        sum+=current->d_value;
        current = current->next;
    }
    return sum;
}

/**
 * Function: init_cost_diff_array
 * ------------------------------
 * This function initializes a cost difference array for a given number of paths.
 *
 * Parameters:
 * - diff_array: A pointer to a double_unsigned_list_t pointer array that will store the cost difference arrays.
 * - nb_paths: The number of paths for which the cost difference arrays will be initialized.
 *
 * Returns:
 * - An integer value representing the status of the function execution. Returns MEMORY_ALLOC_ERROR if memory allocation fails, otherwise returns OK.
 *
 * Note:
 * - The function allocates memory for the cost difference arrays using calloc.
 * - Each cost difference array is initialized as NULL.
 * - The diff_array parameter is updated with the allocated memory.
 *
 * Example usage:
 * ```
 * double_unsigned_list_t **diff_array;
 * unsigned int nb_paths = 5;
 * int status = init_cost_diff_array(&diff_array, nb_paths);
 * if (status == MEMORY_ALLOC_ERROR) {
 *     printf("Memory allocation failed.\n");
 * } else {
 *     printf("Cost difference array initialized successfully.\n");
 * }
 * ```
 */
int init_cost_diff_array(double_unsigned_list_t ***diff_array, unsigned int nb_paths)
{
    *diff_array = calloc(nb_paths, sizeof(double_unsigned_list_t *)); 
    if(*diff_array == NULL){
        return MEMORY_ALLOC_ERROR;
    }
    for (unsigned int i = 0; i < nb_paths; i++)
    {
        (*diff_array)[i] = NULL;
    }
    return OK;
}

/*
 * Function: free_cost_diff_array
 * ------------------------------
 * This function frees the memory allocated for a 2D array of double_unsigned_list_t structures.
 *
 * Parameters:
 * - array: Pointer to a 2D array of double_unsigned_list_t structures.
 * - nb_paths: Number of paths in the array.
 *
 * Returns:
 * - None
 *
 * Note:
 * - This function assumes that the memory for each double_unsigned_list_t structure has already been freed.
 * - The array itself is also freed before returning.
 *
 * Example usage:
 * free_cost_diff_array(array, nb_paths);
 */
void free_cost_diff_array(double_unsigned_list_t **array, uint32_t nb_paths){
    for (size_t path_id = 0; path_id < nb_paths; path_id++)
    {
        free_double_unsigned_list_t(array[path_id]);
    }
    free(array);
}

/**
 * Function: get_max_edge_to_optimize
 * ----------------------------------
 * This function calculates the maximum cost saved by improving an edge and the edge ID to optimize.
 *
 * Parameters:
 * - diff_array: A pointer to a double_unsigned_list_t pointer array containing the cost difference arrays for each path.
 * - nb_paths: The number of paths.
 * - graph: A pointer to the graph structure.
 * - edge_id_to_optimize: A pointer to an integer that will store the edge ID to optimize.
 * - saved_cost: A pointer to a long double that will store the maximum cost saved.
 * - budget_left: The budget left for edge optimization.
 *
 * Returns:
 * - None
 *
 * Note:
 * - This function calculates the cost saved for each edge and stores it in an array.
 * - It then finds the edge with the maximum cost saved and updates the edge_id_to_optimize and saved_cost parameters.
 *
 * Example usage:
 * ```
 * double_unsigned_list_t **diff_array;
 * uint32_t nb_paths = 5;
 * graph_t graph;
 * int edge_id_to_optimize;
 * long double saved_cost;
 * long double budget_left = 100.0;
 * get_max_edge_to_optimize(diff_array, nb_paths, &graph, &edge_id_to_optimize, &saved_cost, budget_left);
 * ```
 */
void get_max_edge_to_optimize(double_unsigned_list_t **diff_array, uint32_t nb_paths, graph_t * graph, int *edge_id_to_optimize, long double *saved_cost, long double budget_left)
{
    long double max_cost_saved = 0;
    uint32_t max_cost_edge_id=-1;
    long double *edge_cost_saved = calloc(graph->nb_edges, sizeof(long double));
    double_unsigned_list_t *temp;

    // We calculate the cost saved for each edge
    for (uint32_t path_id = 0; path_id < nb_paths; path_id++)
    {
        temp = diff_array[path_id];
        while (temp != NULL)
        {
            edge_cost_saved[temp->u_value] += temp->d_value;
            temp = temp->next;
        }
    }

    // We find the edge with the maximum cost saved
    // This is done in two part to optimize the search
    for (uint32_t id_edge = 0; id_edge < graph->nb_edges; id_edge++)
    {
        if ((graph->edge_array[id_edge]->dist <= budget_left) && edge_cost_saved[id_edge] > max_cost_saved)
        {
            max_cost_saved = edge_cost_saved[id_edge];
            max_cost_edge_id = id_edge;
        }
    }

    // zero if no edge can be optimized
    if (max_cost_saved > 0)
    {
        *saved_cost = max_cost_saved;
        *edge_id_to_optimize = max_cost_edge_id;
    }
    free(edge_cost_saved);
}