#include "../header/path.h"
#include <stdbool.h>
#include "../header/util.h"

edge_t* get_edge_if_exist(vertex_t ** graph,uint32_t vertex_source, uint32_t vertex_dest){
    for (uint32_t i = 0; i < graph[vertex_source]->nb_edges_out; i++)
    {
        if(graph[vertex_source]->out_edges[i]->id == vertex_dest){
            return graph[vertex_source]->out_edges[i];
        }
    }
    return NULL;
}

bool vertex_is_in_visibilite(path_t *paths, uint32_t vertex_id)
{

    for (uint32_t i = 0; i < paths->nb_visibilite; i++)
    {
        if (paths->visibilite[i] == vertex_id)
        {
            return true;
        }
    }
    return false;
}

bool edge_is_in_visibilite(path_t *paths, edge_t *edge)
{

    bool succ_in = false;
    bool pred_in = false;

    for (unsigned int i = 0; i < paths->nb_visibilite; i++)
    {
        // if the id of the successor vertex or the predecessor vertex is in the visibility array
        // then returns true
        if (paths->visibilite[i] == edge->succ->id)
        {
            succ_in = true;
        }
        else if (paths->visibilite[i] == edge->pred->id)
        {
            pred_in = true;
        }
        if (pred_in && succ_in)
        {
            break;
        }
    }
    return (succ_in && pred_in);
}

bool troncon_is_in_path(path_t *paths, edge_t *edge)
{
    for (unsigned int i = 0; i < paths->nb_chemin; i++)
    {
        if (paths->chemin[i] == edge->succ->id || paths->chemin[i] == edge->pred->id)
        {
            return true;
        }
    }
    for (unsigned int i = 0; i < paths->nb_visibilite; i++)
    {
        if (paths->visibilite[i] == edge->succ->id || paths->visibilite[i] == edge->pred->id)
        {
            return true;
        }
    }
    return false;
}

int get_paths(cifre_conf_t *conf,path_t ***paths_ref, uint32_t *nb_paths)
{
    uint32_t nb_col, nb_row;
    uint32_t id_offset; // to ignore the header row of the csv file
    char ***csv_matrix;
    int ret_code;

    ret_code = readCSVFile(conf->paths_file_path, &csv_matrix, &nb_row, &nb_col, conf->csv_delimiter);
    if (ret_code != OK)
    {
        return ret_code;
    }

    *paths_ref = (path_t **)calloc(nb_row, sizeof(path_t *));
    path_t **paths = *paths_ref;
    for (uint32_t i = 1; i < nb_row; i++)
    {
        id_offset = i - 1;
        paths[id_offset] = calloc(1, sizeof(path_t));
        paths[id_offset]->distance = atof(csv_matrix[i][conf->path_indexes.distance]);
        paths[id_offset]->danger = atof(csv_matrix[i][conf->path_indexes.danger]);

        paths[id_offset]->profil = atof(csv_matrix[i][conf->path_indexes.profile]);
        paths[id_offset]->origin = atoi(csv_matrix[i][conf->path_indexes.origin]);
        paths[id_offset]->destination = atoi(csv_matrix[i][conf->path_indexes.destination]);
        ret_code = parseJsonIntegerArray(csv_matrix[i][conf->path_indexes.visibility], &paths[id_offset]->visibilite, &paths[id_offset]->nb_visibilite);
        if (ret_code != OK)
        {
            return ret_code;
        }

        ret_code = parseJsonIntegerArray(csv_matrix[i][conf->path_indexes.original_path], &paths[id_offset]->chemin, &paths[id_offset]->nb_chemin);
        if (ret_code != OK)
        {
            return ret_code;
        }

        paths[id_offset]->cps_djikstra_danger = atof(csv_matrix[i][conf->path_indexes.danger_shortest_path]);
        paths[id_offset]->cps_djikstra_dist = atof(csv_matrix[i][conf->path_indexes.distance_shortest_path]);
        ret_code = parseJsonIntegerArray(csv_matrix[i][conf->path_indexes.shortest_path], &paths[id_offset]->djikstra_sp, &paths[id_offset]->nb_djikstra_sp);
        if (ret_code != OK)
        {
            return ret_code;
        }
    }
    freeCSVMatrix(csv_matrix, nb_row, nb_col);
    *nb_paths = (nb_row - 1);
    return OK;
}

void free_paths(path_t **paths, uint32_t nb_paths)
{

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        free(paths[i]->djikstra_sp);
        free(paths[i]->visibilite);
        free(paths[i]->chemin);
        free(paths[i]);
    }
    free(paths);
}