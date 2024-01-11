#ifndef CONFIG_H
#define CONFIG_H
#include "../header/cJSON.h"
#include <stdint.h>

#define CONFIG_FILE_MAX_CHAR 10000

typedef struct {
    uint32_t origin;
    uint32_t destination;
    uint32_t profile;
    uint32_t distance;
    uint32_t danger;
    uint32_t original_path;
    uint32_t shortest_path;
    uint32_t distance_shortest_path;
    uint32_t danger_shortest_path;
    uint32_t visibility;
}path_indexes_t;

typedef struct{
    char * paths_file_path;
    char * graph_file_path;
    char * csv_delimiter;
    long double budget;
    uint32_t thread_number;
    path_indexes_t path_indexes;
}cifre_conf_t;

int get_double_in_json(cJSON *json_object, char *item_key,long double *value);
int get_uint_in_json(cJSON *json_object, char *item_key, uint32_t *value);
int get_string_in_json(cJSON *json_object, char *item_key, char **value);
int set_config(char const *config_file_path,cifre_conf_t *conf_ref);

#endif // CONFIG_H