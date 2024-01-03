#ifndef UTIL_H
#define UTIL_H
#define MAX_LINE_SIZE 350000
#define MAX_FIELDS 14
#include <stdint.h>

enum error_code{
    OK,
    MEMORY_ALLOC_ERROR,
    MEMORY_REALLOC_ERROR,
    FILE_OPENING_ERROR,
    JSON_PARSING_ERROR
};

int readCSVFile(const char *filename,char **** csv_matrix, uint32_t *numRows, uint32_t *numCols, char *delimiter);
uint32_t get_nb_node(char*** csv_matrix, uint32_t nb_row);
void freeCSVMatrix(char ***matrix, int numRows, int numCols);
int parseJsonIntegerArray(const char *json,unsigned int ** result_array_ref,  unsigned int * nb_element);

#endif // UTIL_H