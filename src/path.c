#include "../header/path.h"
#include <stdbool.h>
#include "../header/util.h"


/**
 * Function: vertex_is_in_visibilite
 * ----------------------------------
 * This function checks if a vertex is in the visibility array of a path.
 *
 * paths: The path structure containing the visibility array.
 * vertex_id: The ID of the vertex to check.
 *
 * Returns:
 * - true if the vertex is in the visibility array.
 * - false otherwise.
 *
 * Note:
 * This function uses binary search to efficiently search for the vertex in the visibility array.
 * Make sure the visibility array is sorted in ascending order before calling this function.
 *
 * Example usage:
 * bool result = vertex_is_in_visibilite(&path, 5);
 */
bool vertex_is_in_visibilite(path_t *paths, uint32_t vertex_id) {
    return value_is_in_array(paths->visibilite, paths->nb_visibilite, vertex_id);
}

/**
 * Function: edge_is_in_visibilite
 * ----------------------------------
 * This function checks if both the successor and predecessor vertices of an edge are in the visibility array of a path.
 *
 * Parameters:
 * - paths: The path structure containing the visibility array.
 * - edge: The edge to check.
 *
 * Returns:
 * - true if both the successor and predecessor vertices are in the visibility array.
 * - false otherwise.
 *
 * Note:
 * This function uses binary search to efficiently search for the vertices in the visibility array.
 * Make sure the visibility array is sorted in ascending order before calling this function.
 * 
 * Example usage:
 * bool result = edge_is_in_visibilite(&path, &edge);
 */
bool edge_is_in_visibilite(path_t *paths, edge_t *edge)
{
    bool succ_in = value_is_in_array(paths->visibilite, paths->nb_visibilite, edge->succ->id);
    bool pred_in = value_is_in_array(paths->visibilite, paths->nb_visibilite, edge->pred->id);

    return (succ_in && pred_in);
}

/*
 * Function: get_paths
 * ----------------------------------
 * This function reads a CSV file containing path data and populates an array of path_t structures.
 *
 * Parameters:
 * - conf: a pointer to a config_t structure containing configuration data
 * - paths_ref: a double pointer to a path_t structure array, which will be populated with path data
 * - nb_paths: a pointer to a uint32_t variable, which will be updated with the number of paths read
 *
 * Returns:
 * - OK if the CSV file is successfully read and the path data is populated
 * - an error code if there is an error reading the CSV file or parsing the path data
 *
 * Note:
 * - The function assumes that the CSV file has a header row and the path data starts from the second row.
 * - The function dynamically allocates memory for the path data and the visibility and shortest path arrays.
 * - The caller is responsible for freeing the memory allocated for the path data.
 *
 * Example usage:
 * - int ret_code = get_paths(&conf, &paths, &nb_paths);
 */
int get_paths(config_t *conf,path_t ***paths_ref, uint32_t *nb_paths)
{
    uint32_t nb_col, nb_row;
    uint32_t id_offset; // to ignore the header row of the csv file
    char ***csv_matrix;
    int ret_code;

    ret_code = read_csv_file(conf->paths_file_path, &csv_matrix, &nb_row, &nb_col, conf->csv_delimiter);
    if (ret_code != OK)
    {
        return ret_code;
    }

    *paths_ref = (path_t **)calloc(nb_row, sizeof(path_t *));
    path_t **paths = *paths_ref;

    // ignore the header row
    for (uint32_t i = 1; i < nb_row; i++)
    {
        id_offset = i - 1;
        paths[id_offset] = calloc(1, sizeof(path_t));
        paths[id_offset]->distance = atof(csv_matrix[i][conf->path_indexes.distance]);
        paths[id_offset]->danger = atof(csv_matrix[i][conf->path_indexes.danger]);

        paths[id_offset]->profil = atof(csv_matrix[i][conf->path_indexes.profile]);
        paths[id_offset]->origin = atoi(csv_matrix[i][conf->path_indexes.origin]);
        paths[id_offset]->destination = atoi(csv_matrix[i][conf->path_indexes.destination]);

        //sort the visiblity array so that it we can use Binary Search
        ret_code = parse_and_sort_json_integer_array(csv_matrix[i][conf->path_indexes.visibility], &paths[id_offset]->visibilite, &paths[id_offset]->nb_visibilite);
        if (ret_code != OK)
        {
            return ret_code;
        }

        ret_code = parse_json_integer_array(csv_matrix[i][conf->path_indexes.original_path], &paths[id_offset]->chemin, &paths[id_offset]->nb_chemin);
        if (ret_code != OK)
        {
            return ret_code;
        }

        paths[id_offset]->cps_dijkstra_danger = atof(csv_matrix[i][conf->path_indexes.danger_shortest_path]);
        paths[id_offset]->cps_dijkstra_dist = atof(csv_matrix[i][conf->path_indexes.distance_shortest_path]);
        ret_code = parse_json_integer_array(csv_matrix[i][conf->path_indexes.shortest_path], &paths[id_offset]->dijkstra_sp, &paths[id_offset]->nb_dijkstra_sp);
        if (ret_code != OK)
        {
            return ret_code;
        }
    }
    free_csv_matrix(csv_matrix, nb_row, nb_col);
    *nb_paths = (nb_row - 1);
    return OK;
}

/*
 * Function: free_paths
 * ----------------------------------
 * This function frees the memory allocated for an array of path_t structures.
 *
 * paths: The array of path_t structures.
 * nb_paths: The number of paths in the array.
 *
 * Returns: void
 *
 * Note: This function should be called before returning.
 *
 * Example usage: 
 * free_paths(paths, nb_paths);
 */
void free_paths(path_t **paths, uint32_t nb_paths)
{
    for (uint32_t i = 0; i < nb_paths; i++)
    {
        free(paths[i]->dijkstra_sp);
        free(paths[i]->visibilite);
        free(paths[i]->chemin);
        free(paths[i]);
    }
    free(paths);
}