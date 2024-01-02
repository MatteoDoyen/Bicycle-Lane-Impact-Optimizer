#ifndef UTIL_H
#define UTIL_H
#define MAX_LINE_SIZE 350000
#define MAX_FIELDS 14
#include <stdint.h>

char ***readCSVFile(const char *filename, uint32_t *numRows, uint32_t *numCols, char *delimiter);
uint32_t get_nb_node(char*** csv_matrix, uint32_t nb_row);
void freeCSVMatrix(char ***matrix, int numRows, int numCols);
unsigned int * parseJsonIntegerArray(const char *json,  unsigned int * nb_element);

#endif // UTIL_H