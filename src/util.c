#include "../header/util.h"
#include "../header/cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char ***readCSVFile(const char *filename, uint32_t *numRows, uint32_t *numCols, char *delimiter)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return NULL;
    }

    // Count the number of rows and columns
    *numRows = 0;
    *numCols = 0;
    // char *line = malloc(sizeof(char)*MAX_LINE_SIZE);
    char line[MAX_LINE_SIZE];
    // int32_t max_line_size=0;
    while (fgets(line, MAX_LINE_SIZE, file) != NULL)
    {
        (*numRows)++;
        char *token = strtok(line, delimiter);
        uint32_t colCount = 0;

        while (token != NULL && colCount < MAX_FIELDS)
        {
            colCount++;
            token = strtok(NULL, delimiter);
        }

        if (colCount > *numCols)
        {
            *numCols = colCount;
        }
    }

    // Rewind the file to read it again
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the matrix
    char ***matrix = (char ***)calloc(1,*numRows * sizeof(char **));
    for (uint32_t i = 0; i < *numRows; ++i)
    {
        matrix[i] = (char **)calloc(1,*numCols * sizeof(char *));
        for (uint32_t j = 0; j < *numCols; ++j)
        {
            // matrix[i][j] = (char *)calloc(1,MAX_LINE_SIZE * sizeof(char));
            matrix[i][j] = NULL;
        }
    }

    // Read the CSV file and populate the matrix
    uint32_t row = 0;
    while (fgets(line, MAX_LINE_SIZE, file) != NULL && row < *numRows)
    {
        char *token = strtok(line, delimiter);
        uint32_t col = 0;

        while (token != NULL && col < *numCols)
        {
            // max_line_size = ((int32_t)strlen(token)) >max_line_size ?((int32_t)strlen(token)) :max_line_size; 
            matrix[row][col] = (char *)calloc(1,(strlen(token)+1) * sizeof(char));
            strcpy(matrix[row][col], token);
            col++;
            token = strtok(NULL, delimiter);
        }

        row++;
    }
    // printf("max line : %d\n",max_line_size);
    // free(line);
    fclose(file);
    return matrix;
}

void freeCSVMatrix(char ***matrix, int32_t numRows, int32_t numCols)
{
    for (int32_t i = 0; i < numRows; ++i)
    {
        for (int32_t j = 0; j < numCols; ++j)
        {
            if(matrix[i][j]!=NULL){
                free(matrix[i][j]);
            }
        }
        free(matrix[i]);
    }
    free(matrix);
}

 unsigned int *parseJsonIntegerArray(const char *json, unsigned int *nb_element)
{
    // fprintf(stderr,"eueueu\n");
    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        fprintf(stderr, "Error parsing JSON.\n");
        return NULL;
    }

    if (!cJSON_IsArray(root))
    {
        fprintf(stderr, "JSON is not an array.\n");
        cJSON_Delete(root);
        return NULL;
    }

    int arraySize = cJSON_GetArraySize(root);
    *nb_element = arraySize;
    unsigned int *resultArray = ( unsigned int *)calloc(1,arraySize * sizeof( unsigned int));

    if (!resultArray)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        cJSON_Delete(root);
        return NULL;
    }

    for (int i = 0; i < arraySize; ++i)
    {
        cJSON *item = cJSON_GetArrayItem(root, i);
        if (!cJSON_IsNumber(item))
        {
            fprintf(stderr, "Array element at index %d is not a number.\n", i);
            free(resultArray);
            cJSON_Delete(root);
            return NULL;
        }

        resultArray[i] = item->valueint;
    }

    cJSON_Delete(root);
    // fprintf(stderr,"out\n");
    return resultArray;
}