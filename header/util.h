#ifndef UTIL_H
#define UTIL_H
#define MAX_LINE_SIZE 4000
#define MAX_FIELDS 14

char ***readCSVFile(const char *filename, int *numRows, int *numCols, char *delimiter);
unsigned int get_nb_node(char*** csv_matrix, int nb_row);
void freeCSVMatrix(char ***matrix, int numRows, int numCols);
unsigned int * parseJsonIntegerArray(const char *json,  unsigned int * nb_element);

#endif