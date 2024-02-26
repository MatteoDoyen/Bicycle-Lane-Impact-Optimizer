#include "../header/config.h"
#include "../header/util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * Function: get_config
 * --------------------
 * This function reads a configuration file and populates a config_t structure with the parsed values.
 *
 * Parameters:
 * - config_file_path: The path to the configuration file.
 * - conf_ref: A pointer to the config_t structure to be populated.
 *
 * Returns:
 * - OK if the configuration file was successfully parsed and the structure populated.
 * - FILE_OPENING_ERROR if there was an error opening the configuration file.
 * - JSON_PARSING_ERROR if there was an error parsing the JSON data in the configuration file.
 *
 * Note:
 * - The config_t structure must be allocated before calling this function.
 * - The function assumes that the configuration file is a valid JSON file with the expected structure.
 * - The function uses cJSON library for JSON parsing.
 * - The function reads the JSON data from the configuration file, extracts the required values, and assigns them to the corresponding fields in the config_t structure.
 * - The function returns an error code if any of the required values are missing or have an invalid format.
 *
 * Example usage:
 * ```
 * config_t conf;
 * int result = get_config("/path/to/config.json", &conf);
 * if (result == OK) {
 *     // Configuration file was successfully parsed and the structure was populated
 *     // Use the values in the conf structure
 * } else {
 *     // Error occurred while parsing the configuration file
 *     // Handle the error
 * }
 * ```
 */
int get_config(char const *config_file_path, config_t *conf_ref)
{
    cJSON *path_indexes_dict;
    cJSON *graph_indexes_dict;
    char buffer[CONFIG_FILE_MAX_CHAR];

    FILE *fp = fopen(config_file_path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening the file");
        return FILE_OPENING_ERROR;
    }
    // Read the file into a buffer
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);

    if (bytesRead == 0 && ferror(fp))
    {
        fprintf(stderr, "Error reading the file\n");
        return FILE_READING_ERROR;
    }
    else if (bytesRead == 0 && feof(fp))
    {
        fprintf(stderr, "Error: file is empty\n");
        return FILE_EMPTY_ERROR;
    }
    else
    {
        buffer[bytesRead] = '\0'; // Null-terminate the string in the buffer
    }

    cJSON *json = cJSON_Parse(buffer);

    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error : %s\n", error_ptr);
        }
        cJSON_Delete(json);
        return JSON_PARSING_ERROR;
    }

    // Get the dict containing the path indexes
    path_indexes_dict = cJSON_GetObjectItem(json, "path_column_indexes");
    if (!cJSON_IsObject(path_indexes_dict))
    {
        fprintf(stderr, "Error : dict path_column_indexes does not exist\n");
        cJSON_Delete(json);
        return JSON_PARSING_ERROR;
    }
    // Get the values from the dict and assign them to the corresponding fields in the config_t structure
    get_uint_in_json(path_indexes_dict, "origin", &(*conf_ref).path_indexes.origin);
    get_uint_in_json(path_indexes_dict, "destination", &(*conf_ref).path_indexes.destination);
    get_uint_in_json(path_indexes_dict, "profile", &(*conf_ref).path_indexes.profile);
    get_uint_in_json(path_indexes_dict, "distance", &(*conf_ref).path_indexes.distance);
    get_uint_in_json(path_indexes_dict, "danger", &(*conf_ref).path_indexes.danger);
    get_uint_in_json(path_indexes_dict, "original_path", &(*conf_ref).path_indexes.original_path);
    get_uint_in_json(path_indexes_dict, "shortest_path", &(*conf_ref).path_indexes.shortest_path);
    get_uint_in_json(path_indexes_dict, "distance_shortest_path", &(*conf_ref).path_indexes.distance_shortest_path);
    get_uint_in_json(path_indexes_dict, "danger_shortest_path", &(*conf_ref).path_indexes.danger_shortest_path);
    get_uint_in_json(path_indexes_dict, "visibility", &(*conf_ref).path_indexes.visibility);

    graph_indexes_dict = cJSON_GetObjectItem(json, "graph_column_indexes");
    if (!cJSON_IsObject(graph_indexes_dict))
    {
        fprintf(stderr, "Error : dict graph_column_indexes does not exist\n");
        cJSON_Delete(json);
        return JSON_PARSING_ERROR;
    }
    // Get the values from the dict and assign them to the corresponding fields in the config_t structure
    get_uint_in_json(graph_indexes_dict, "edge_id", &(*conf_ref).graph_indexes.edge_id);
    get_uint_in_json(graph_indexes_dict, "node_i", &(*conf_ref).graph_indexes.node_i);
    get_uint_in_json(graph_indexes_dict, "node_j", &(*conf_ref).graph_indexes.node_j);
    get_uint_in_json(graph_indexes_dict, "distance", &(*conf_ref).graph_indexes.distance);
    get_uint_in_json(graph_indexes_dict, "danger", &(*conf_ref).graph_indexes.danger);
    get_uint_in_json(graph_indexes_dict, "amenagement", &(*conf_ref).graph_indexes.amenagement);

    // These values are at the root of the JSON file
    get_string_in_json(json, "paths_file_path", &(*conf_ref).paths_file_path);
    get_string_in_json(json, "graph_file_path", &(*conf_ref).graph_file_path);
    get_string_in_json(json, "csv_delimiter", &(*conf_ref).csv_delimiter);
    get_double_in_json(json, "budget", &(*conf_ref).budget);
    get_uint_in_json(json, "thread_number", &(*conf_ref).thread_number);

    cJSON_Delete(json);

    return OK;
}

/*
 * Function: free_config
 * ---------------------
 * This function frees the memory allocated for the config structure.
 *
 * Parameters:
 * - config: A pointer to the config_t structure to be freed.
 *
 * Returns:
 * This function does not return a value.
 *
 * Note:
 * Make sure to call this function to free the memory allocated for the config structure before returning.
 *
 * Example usage:
 * config_t myConfig;
 * // code to initialize myConfig
 * free_config(&myConfig);
 */
void free_config(config_t *config)
{
    free(config->paths_file_path);
    free(config->graph_file_path);
    free(config->csv_delimiter);
}

/*
 * Function: get_double_in_json
 * ----------------------------
 * This function retrieves a double value from a JSON object based on the specified item key.
 *
 * Parameters:
 * - json_object: A pointer to the cJSON object.
 * - item_key: The key of the item to retrieve.
 * - value: A pointer to the variable where the retrieved double value will be stored.
 *
 * Returns:
 * - 0 if the retrieval is successful, 1 otherwise.
 *
 * Note:
 * - Make sure to call cJSON_Delete(json_object) to free the memory allocated for the cJSON object before returning.
 */

int get_double_in_json(cJSON *json_object, char *item_key, long double *value)
{
    cJSON *current_item = cJSON_GetObjectItem(json_object, item_key);
    if (current_item == NULL || !cJSON_IsNumber(current_item))
    {
        fprintf(stderr, "Error : item %s should be a number\n", item_key);
        cJSON_Delete(json_object);
        return 1;
    }
    *value = current_item->valuedouble;
    return 0;
}

/*
 * Function: get_uint_in_json
 * --------------------------
 * This function retrieves an unsigned integer value from a JSON object based on the specified item key.
 *
 * Parameters:
 * - json_object: A pointer to the cJSON object.
 * - item_key: The key of the item to retrieve.
 * - value: A pointer to the variable where the retrieved unsigned integer value will be stored.
 *
 * Returns:
 * - 0 if the retrieval is successful, 1 otherwise.
 *
 * Note:
 * - Make sure to call cJSON_Delete(json_object) to free the memory allocated for the cJSON object before returning.
 */

int get_uint_in_json(cJSON *json_object, char *item_key, uint32_t *value)
{
    cJSON *current_item = cJSON_GetObjectItem(json_object, item_key);
    if (current_item == NULL || !cJSON_IsNumber(current_item))
    {
        fprintf(stderr, "Error : item %s should be a number\n", item_key);
        cJSON_Delete(json_object);
        return 1;
    }
    *value = (uint32_t)current_item->valueint;
    return 0;
}

/*
 * Function: get_string_in_json
 * ----------------------------
 * This function retrieves a string value from a JSON object based on the specified item key.
 *
 * Parameters:
 * - json_object: A pointer to the cJSON object.
 * - item_key: The key of the item to retrieve.
 * - value: A pointer to the variable where the retrieved string value will be stored.
 *
 * Returns:
 * - 0 if the retrieval is successful, 1 otherwise.
 *
 * Note:
 * - Make sure to call cJSON_Delete(json_object) to free the memory allocated for the cJSON object before returning.
 */

int get_string_in_json(cJSON *json_object, char *item_key, char **value)
{
    cJSON *current_item = cJSON_GetObjectItem(json_object, item_key);
    if (current_item == NULL || !cJSON_IsString(current_item) || current_item->valuestring == NULL)
    {
        fprintf(stderr, "Error : item %s should be a string\n", item_key);
        cJSON_Delete(json_object);
        return 1;
    }
    *value = strdup(current_item->valuestring);
    return 0;
}
