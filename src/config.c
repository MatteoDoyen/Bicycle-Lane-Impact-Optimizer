#include "../header/config.h"
#include "../header/util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int set_config(char const *config_file_path,cifre_conf_t *conf_ref)
{
    cJSON *path_indexes_dict;

    FILE *fp = fopen(config_file_path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening the file");
        return FILE_OPENING_ERROR;
    }
    char buffer[CONFIG_FILE_MAX_CHAR];
    fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);
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
    path_indexes_dict = cJSON_GetObjectItem(json, "path_column_indexes");
    if (!cJSON_IsObject(path_indexes_dict))
    {
        fprintf(stderr, "Error : item should be object\n");
        cJSON_Delete(json);
        return JSON_PARSING_ERROR;
    }
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

    get_string_in_json(json, "paths_file_path", &(*conf_ref).paths_file_path);
    get_string_in_json(json, "graph_file_path", &(*conf_ref).graph_file_path);
    get_string_in_json(json, "csv_delimiter", &(*conf_ref).csv_delimiter);
    get_double_in_json(json, "budget", &(*conf_ref).budget);
    get_uint_in_json(json, "thread_number", &(*conf_ref).thread_number);

    cJSON_Delete(json);

    return OK;
}

void free_config(cifre_conf_t *config){
    free(config->paths_file_path);
    free(config->graph_file_path);
    free(config->csv_delimiter);
}

int get_string_in_json(cJSON *json_object, char *item_key, char **value)
{
    cJSON *current_item = cJSON_GetObjectItem(json_object, item_key);
    if (current_item == NULL || !cJSON_IsString(current_item) || current_item->valuestring == NULL)
    {
        fprintf(stderr, "Error : item should be a number\n");
        cJSON_Delete(json_object);
        return 1;
    }
    *value = strdup(current_item->valuestring);
    return 0;
}

int get_double_in_json(cJSON *json_object, char *item_key, long double *value)
{
    cJSON *current_item = cJSON_GetObjectItem(json_object, item_key);
    if (current_item == NULL || !cJSON_IsNumber(current_item))
    {
        fprintf(stderr, "Error : item should be a number\n");
        cJSON_Delete(json_object);
        return 1;
    }
    *value = current_item->valuedouble;
    return 0;
}

int get_uint_in_json(cJSON *json_object, char *item_key, uint32_t *value)
{
    cJSON *current_item = cJSON_GetObjectItem(json_object, item_key);
    if (current_item == NULL || !cJSON_IsNumber(current_item))
    {
        fprintf(stderr, "Error : item should be a number\n");
        cJSON_Delete(json_object);
        return 1;
    }
    *value = (uint32_t) current_item->valueint;
    return 0;
}