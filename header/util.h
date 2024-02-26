#ifndef UTIL_H
#define UTIL_H
#define MAX_LINE_SIZE 350000
#define MAX_FIELDS 14
#include <stdint.h>
#include <stdbool.h>

typedef struct double_unsigned_list_t{
    uint32_t u_value;
    long double d_value;
    struct double_unsigned_list_t *next;
} double_unsigned_list_t;

typedef struct unsigned_list_t
{
    uint32_t u_value;
    struct unsigned_list_t *next;
} unsigned_list_t;

enum error_code{
    OK,
    MEMORY_ALLOC_ERROR,
    MEMORY_REALLOC_ERROR,
    FILE_OPENING_ERROR,
    FILE_READING_ERROR,
    FILE_EMPTY_ERROR,
    JSON_PARSING_ERROR
};

int add_double_unsigned_list_t(double_unsigned_list_t **head, unsigned u_value, long double d_value);
void free_double_unsigned_list_t(double_unsigned_list_t *head);

void delete_value_in_unsigned_list(unsigned_list_t **head, unsigned u_value);
int add_unsigned_list_t(unsigned_list_t **head, unsigned u_value);
void free_unsigned_list_t(unsigned_list_t *head);


int read_csv_file(const char *filename,char **** csv_matrix, uint32_t *numRows, uint32_t *numCols, char *delimiter);
uint32_t get_nb_list_t(char*** csv_matrix, uint32_t nb_row);
void free_csv_matrix(char ***matrix, int numRows, int numCols);
int parse_json_integer_array(const char *json,unsigned int ** result_array_ref,  unsigned int * nb_element);

bool value_is_in_array(uint32_t *arr, uint32_t size, uint32_t target);
int parse_and_sort_json_integer_array(const char *json, unsigned int **result_array_ref, unsigned int *nb_element);
void quick_sort(unsigned int arr[], int low, int high);
int partition(unsigned int arr[], int low, int high);
void swap(unsigned int *a, unsigned int *b);

#endif // UTIL_H