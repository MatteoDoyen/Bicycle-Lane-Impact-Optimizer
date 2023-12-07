#include "../header/util.h"
#include "../header/cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char ***readCSVFile(const char *filename, int *numRows, int *numCols, char *delimiter)
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
    char line[MAX_LINE_SIZE];

    while (fgets(line, MAX_LINE_SIZE, file) != NULL)
    {
        (*numRows)++;
        char *token = strtok(line, delimiter);
        int colCount = 0;

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
    char ***matrix = (char ***)malloc(*numRows * sizeof(char **));
    for (int i = 0; i < *numRows; ++i)
    {
        matrix[i] = (char **)malloc(*numCols * sizeof(char *));
        for (int j = 0; j < *numCols; ++j)
        {
            matrix[i][j] = (char *)malloc(MAX_LINE_SIZE * sizeof(char));
        }
    }

    // Read the CSV file and populate the matrix
    int row = 0;
    while (fgets(line, MAX_LINE_SIZE, file) != NULL && row < *numRows)
    {
        char *token = strtok(line, delimiter);
        int col = 0;

        while (token != NULL && col < *numCols)
        {
            strcpy(matrix[row][col], token);
            col++;
            token = strtok(NULL, delimiter);
        }

        row++;
    }

    fclose(file);
    return matrix;
}

void freeCSVMatrix(char ***matrix, int numRows, int numCols)
{
    for (int i = 0; i < numRows; ++i)
    {
        for (int j = 0; j < numCols; ++j)
        {
            free(matrix[i][j]);
        }
        free(matrix[i]);
    }
    free(matrix);
}

 unsigned int *parseJsonIntegerArray(const char *json, unsigned int *nb_element)
{
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
    unsigned int *resultArray = ( unsigned int *)malloc(arraySize * sizeof( unsigned int));

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
    return resultArray;
}