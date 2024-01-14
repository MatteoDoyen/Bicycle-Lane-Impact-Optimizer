#include "../header/util.h"
#include "../header/cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int readCSVFile(const char *filename, char ****csv_matrix_ref, uint32_t *numRows, uint32_t *numCols, char *delimiter)
{
    char ***matrix;
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file: %s\n", filename);
        return FILE_OPENING_ERROR;
    }

    // Count the number of rows and columns
    *numRows = 0;
    *numCols = 0;
    char line[MAX_LINE_SIZE];
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
    *csv_matrix_ref = (char ***)calloc(*numRows, sizeof(char **));
    if (*csv_matrix_ref == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return MEMORY_ALLOC_ERROR;
    }
    matrix = *csv_matrix_ref;
    for (uint32_t i = 0; i < *numRows; ++i)
    {
        matrix[i] = (char **)calloc(*numCols, sizeof(char *));
        if (matrix[i] == NULL)
        {
            for (uint32_t x = 0; x < i; x++)
            {
                free(matrix[x]);
            }
            free(matrix);
            fprintf(stderr, "Memory allocation failed\n");
            return MEMORY_ALLOC_ERROR;
        }
        for (uint32_t j = 0; j < *numCols; ++j)
        {
            matrix[i][j] = NULL;
        }
    }

    // Read the CSV file and populate the csv_matrix
    uint32_t row = 0;
    while (fgets(line, MAX_LINE_SIZE, file) != NULL && row < *numRows)
    {
        char *token = strtok(line, delimiter);
        uint32_t col = 0;

        while (token != NULL && col < *numCols)
        {
            // max_line_size = ((int32_t)strlen(token)) >max_line_size ?((int32_t)strlen(token)) :max_line_size;
            matrix[row][col] = (char *)calloc((strlen(token) + 1), sizeof(char));
            if (matrix[row][col] == NULL)
            {
                // free all the cols for the previous rows
                // assumes they all had a numCols number of rows
                for (uint32_t i = 0; i < *numRows; i++)
                {
                    for (uint32_t x = 0; x < *numCols; x++)
                    {
                        free(matrix[i][x]);
                    }
                }
                // free all the col for the current row
                for (uint32_t i = 0; i < col; i++)
                {
                    free(matrix[row][i]);
                }
                fprintf(stderr, "Memory allocation failed\n");
                return MEMORY_ALLOC_ERROR;
            }
            strcpy(matrix[row][col], token);
            col++;
            token = strtok(NULL, delimiter);
        }

        row++;
    }
    // printf("max line : %d\n",max_line_size);
    // free(line);
    fclose(file);
    // return matrix;
    return OK;
}

void freeCSVMatrix(char ***matrix, int32_t numRows, int32_t numCols)
{
    for (int32_t i = 0; i < numRows; ++i)
    {
        for (int32_t j = 0; j < numCols; ++j)
        {
            if (matrix[i][j] != NULL)
            {
                free(matrix[i][j]);
            }
        }
        free(matrix[i]);
    }
    free(matrix);
}

int parseJsonIntegerArray(const char *json, unsigned int **result_array_ref, unsigned int *nb_element)
{
    // fprintf(stderr,"eueueu\n");
    unsigned int *resultArray;
    cJSON *root = cJSON_Parse(json);
    if (!root)
    {
        fprintf(stderr, "Error parsing JSON.\n");
        return JSON_PARSING_ERROR;
    }

    if (!cJSON_IsArray(root))
    {
        fprintf(stderr, "JSON is not an array.\n");
        cJSON_Delete(root);
        return JSON_PARSING_ERROR;
    }

    int arraySize = cJSON_GetArraySize(root);
    *nb_element = arraySize;
    *result_array_ref = (unsigned int *)calloc(arraySize, sizeof(unsigned int));
    if (*result_array_ref == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return MEMORY_ALLOC_ERROR;
    }
    resultArray = *result_array_ref;

    if (!resultArray)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        cJSON_Delete(root);
        fprintf(stderr, "Memory allocation failed\n");
        return MEMORY_ALLOC_ERROR;
    }

    for (int i = 0; i < arraySize; ++i)
    {
        cJSON *item = cJSON_GetArrayItem(root, i);
        if (!cJSON_IsNumber(item))
        {
            fprintf(stderr, "Array element at index %d is not a number.\n", i);
            free(resultArray);
            cJSON_Delete(root);
            return JSON_PARSING_ERROR;
        }

        resultArray[i] = item->valueint;
    }

    cJSON_Delete(root);
    return OK;
}