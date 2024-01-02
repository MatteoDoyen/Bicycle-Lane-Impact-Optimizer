#include "../header/path.h"
#include <stdbool.h>
#include "../header/util.h"

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
            succ_in=true;
        }
        else if (paths->visibilite[i] == edge->pred->id)
        {
            pred_in=true;
        }
        if(pred_in && succ_in){
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
            // printf("chem %d succ %d pred %d\n",paths->visibilite[i],edge->succ->id,edge->pred->id);
            return true;
        }
    }
    return false;
}

void get_paths(char *csv_path, char *csv_delimiter,path_t *** paths_ref, uint32_t *nb_paths)
{
    uint32_t nb_col, nb_row;
    uint32_t id_offset; //to ignore the header row of the csv file

    // fprintf(stderr,"before alloc\n");
    char ***csv_matrix = readCSVFile(csv_path, &nb_row, &nb_col, csv_delimiter);
    *paths_ref = (path_t **)calloc(nb_row,sizeof(path_t *));
    path_t **paths = *paths_ref;
    for (uint32_t i = 1; i < nb_row; i++)
    {
        id_offset = i-1;
        paths[id_offset] = calloc(1,sizeof(path_t));
        paths[id_offset]->distance = atof(csv_matrix[i][T_DISTANCE_INDEX]);
        paths[id_offset]->danger = atof(csv_matrix[i][T_DANGER_INDEX]);
        
        paths[id_offset]->profil = atof(csv_matrix[i][T_PROFIL_INDEX]);
        paths[id_offset]->origin = atoi(csv_matrix[i][T_ORIGIN_INDEX]);
        paths[id_offset]->destination = atoi(csv_matrix[i][T_DESTINATION_INDEX]);
        // printf("%s",csv_matrix[i][T_VISIBILITE_INDEX]);
        // break;
        paths[id_offset]->visibilite = parseJsonIntegerArray(csv_matrix[i][T_VISIBILITE_INDEX], &paths[id_offset]->nb_visibilite);
        paths[id_offset]->chemin = parseJsonIntegerArray(csv_matrix[i][T_CHEMIN_INDEX], &paths[id_offset]->nb_chemin);
        paths[id_offset]->cps_djikstra_danger = atof(csv_matrix[i][T_DANGER_CPS_INDEX]);
        paths[id_offset]->cps_djikstra_dist = atof(csv_matrix[i][T_DIST_CPS_INDEX]);
        paths[id_offset]->djikstra_sp = parseJsonIntegerArray(csv_matrix[i][T_CPC_INDEX],&paths[id_offset]->nb_djikstra_sp);
        paths[id_offset]->foward_djikstra = NULL;
        paths[id_offset]->backward_djikstra = NULL;
    }
    // fprintf(stderr,"after\n");
    freeCSVMatrix(csv_matrix, nb_row, nb_col);
    *nb_paths = (nb_row - 1);
}

void free_paths(path_t** paths,uint32_t nb_paths){

    for (uint32_t i = 0; i < nb_paths; i++)
    {
        free(paths[i]->djikstra_sp);
        free(paths[i]->visibilite);
        free(paths[i]->chemin);
        free(paths[i]);
    }
    free(paths);
}