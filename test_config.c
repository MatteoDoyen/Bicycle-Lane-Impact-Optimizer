#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./header/cJSON.h"

typedef struct {
    unsigned int origin;
    unsigned int destination;
    unsigned int profile;
    unsigned int distance;
    unsigned int danger;
    unsigned int original_path;
    unsigned int shortest_path;
    unsigned int distance_shortest_path;
    unsigned int danger_shortest_path;
    unsigned int visibility;
}path_indexes_t;

typedef struct{
    char * paths_file_path;
    char * graph_file_path;
    double budget;
    unsigned int thread_number;
    path_indexes_t path_indexes;
}cifre_conf_t;


int get_string_in_json(cJSON * json_object,char * item_key,char ** value){
    cJSON * current_item = cJSON_GetObjectItem(json_object,item_key);
    if(current_item==NULL || !cJSON_IsString(current_item) || current_item->valuestring == NULL){
        fprintf(stderr,"Error : item should be a number\n");
        cJSON_Delete(json_object);
        return 1;
    }
    *value = strdup(current_item->valuestring);
    return 0;
}


int get_double_in_json(cJSON * json_object,char * item_key,double * value){
    cJSON * current_item = cJSON_GetObjectItem(json_object,item_key);
    if(current_item==NULL || !cJSON_IsNumber(current_item)){
        fprintf(stderr,"Error : item should be a number\n");
        cJSON_Delete(json_object);
        return 1;
    }
    *value = current_item->valuedouble;
    return 0;
}

int get_int_in_json(cJSON * json_object,char * item_key,int * value){
    cJSON * current_item = cJSON_GetObjectItem(json_object,item_key);
    if(current_item==NULL || !cJSON_IsNumber(current_item)){
        fprintf(stderr,"Error : item should be a number\n");
        cJSON_Delete(json_object);
        return 1;
    }
    *value = current_item->valueint;
    return 0;
}

int main(int argc, char const *argv[])
{
    cifre_conf_t conf;
    cJSON * path_indexes_dict, * current_item;
    
    FILE * fp = fopen("./conf/conf_real.json","r");
    if (fp ==NULL)
    {
        fprintf(stderr,"Error opening the file");
        return 1;
    }
    char buffer[10000];
    int len = fread(buffer,1,sizeof(buffer),fp);
    fclose(fp);
    cJSON * json = cJSON_Parse(buffer);
    if(json==NULL){
        const char * error_ptr = cJSON_GetErrorPtr();
        if(error_ptr!=NULL){
            fprintf(stderr,"Error : %s\n",error_ptr);
        }
        cJSON_Delete(json);
        return 1;
    }
    path_indexes_dict = cJSON_GetObjectItem(json,"path_column_indexes");
    if(!cJSON_IsObject(path_indexes_dict)){
        fprintf(stderr,"Error : item should be object\n");
        cJSON_Delete(json);
        return 1;
    }
    get_int_in_json(path_indexes_dict,"origin",&conf.path_indexes.origin);
    get_int_in_json(path_indexes_dict,"destination",&conf.path_indexes.destination);
    get_int_in_json(path_indexes_dict,"profile",&conf.path_indexes.profile);
    get_int_in_json(path_indexes_dict,"distance",&conf.path_indexes.distance);
    get_int_in_json(path_indexes_dict,"danger",&conf.path_indexes.danger);
    get_int_in_json(path_indexes_dict,"original_path",&conf.path_indexes.original_path);
    get_int_in_json(path_indexes_dict,"shortest_path",&conf.path_indexes.shortest_path);
    get_int_in_json(path_indexes_dict,"distance_shortest_path",&conf.path_indexes.distance_shortest_path);
    get_int_in_json(path_indexes_dict,"danger_shortest_path",&conf.path_indexes.danger_shortest_path);
    get_int_in_json(path_indexes_dict,"visibility",&conf.path_indexes.visibility);
    
    get_string_in_json(json,"paths_file_path",&conf.paths_file_path);
    get_string_in_json(json,"graph_file_path",&conf.graph_file_path);
    get_double_in_json(json,"budget",&conf.budget);
    get_int_in_json(json,"thread_number",&conf.thread_number);

    printf("paths_file_path : %s\n",conf.paths_file_path);

    cJSON_Delete(json);
    
    return 0;
}
