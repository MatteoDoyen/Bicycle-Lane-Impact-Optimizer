#include "../header/trace.h"
#include <stdbool.h>
#include "../header/util.h"

bool vertexIsInVisiblite(trace_t *traces, unsigned int vertex_id)
{

    for (unsigned int i = 0; i < traces->nb_visibilite; i++)
    {
        if (traces->visibilite[i] == vertex_id)
        {
            return true;
        }
    }
    return false;
}

bool arcIsInVisiblite(trace_t *traces, Arc *arc)
{

    bool succ_in = false;
    bool pred_in = false;

    for (unsigned int i = 0; i < traces->nb_visibilite; i++)
    {
        // if the id of the successor vertex or the predecessor vertex is in the visibility array
        // then returns true
        if (traces->visibilite[i] == arc->succ->id)
        {
            succ_in=true;
        }
        else if (traces->visibilite[i] == arc->predecesseur->id)
        {
            pred_in=true;
        }
        if(pred_in && succ_in){
            break;
        }
    }
    return (succ_in && pred_in);
}

bool tronconIsInTrace(trace_t *traces, Arc *arc)
{
    // printf("là\n");
    for (unsigned int i = 0; i < traces->nb_chemin; i++)
    {
        if (traces->chemin[i] == arc->succ->id || traces->chemin[i] == arc->predecesseur->id)
        {
            // printf("chem %d succ %d pred%d\n",traces->chemin[i],arc->succ->id,arc->predecesseur->id);
            // printf("origin %d dest %d id %d\n",traces->origin,traces->destination,arc->id);
            return true;
        }
    }
    for (unsigned int i = 0; i < traces->nb_visibilite; i++)
    {
        if (traces->visibilite[i] == arc->succ->id || traces->visibilite[i] == arc->predecesseur->id)
        {
            // printf("chem %d succ %d pred %d\n",traces->visibilite[i],arc->succ->id,arc->predecesseur->id);
            return true;
        }
    }
    return false;
}

trace_t *get_traces(char *csv_path, char *csv_delimiter, int *nb_traces)
{

    int nb_col, nb_row;
    char ***csv_matrix = readCSVFile(csv_path, &nb_row, &nb_col, csv_delimiter);
    trace_t *traces = malloc(sizeof(trace_t) * nb_row);
    memset(traces, 0, sizeof(trace_t) * nb_row);
    printf("%d %d\n", nb_col, nb_row);
    for (int i = 1; i < nb_row; i++)
    {
        // csv_matrix = csv_line_to_tab(tmp, 8, DELIMITER);
        traces[i - 1].distance = atof(csv_matrix[i][T_DISTANCE_INDEX]);
        traces[i - 1].danger = atof(csv_matrix[i][T_DANGER_INDEX]);
        traces[i - 1].profil = atof(csv_matrix[i][T_PROFIL_INDEX]);
        traces[i - 1].origin = atoi(csv_matrix[i][T_ORIGIN_INDEX]);
        traces[i - 1].destination = atoi(csv_matrix[i][T_DESTINATION_INDEX]);
        traces[i - 1].visibilite = parseJsonIntegerArray(csv_matrix[i][T_VISIBILITE_INDEX], &traces[i - 1].nb_visibilite);
        traces[i - 1].chemin = parseJsonIntegerArray(csv_matrix[i][T_CHEMIN_INDEX], &traces[i - 1].nb_chemin);
        traces[i - 1].cps_djikstra_danger = atof(csv_matrix[i][T_DANGER_CPS_INDEX]);
        traces[i - 1].cps_djikstra_dist = atof(csv_matrix[i][T_DIST_CPS_INDEX]);
        traces[i - 1].djikstra_sp = parseJsonIntegerArray(csv_matrix[i][T_CPC_INDEX],&traces[i - 1].nb_djikstra_sp);
    }
    freeCSVMatrix(csv_matrix, nb_row, nb_col);
    *nb_traces = (nb_row - 1);
    return traces;
}