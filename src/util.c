#include "../header/util.h"
#include "../header/cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/**
 * Function: read_csv_file
 * ----------------------------------
 * This function reads a CSV file and populates a matrix with its contents.
 *
 * Parameters:
 * - filename: The name of the CSV file to read.
 * - csv_matrix_ref: A reference to the matrix that will store the CSV data.
 * - numRows: A pointer to a variable that will store the number of rows in the CSV file.
 * - numCols: A pointer to a variable that will store the number of columns in the CSV file.
 * - delimiter: The delimiter character used in the CSV file.
 *
 * Returns:
 * - OK if the CSV file was successfully read and the matrix was populated.
 * - FILE_OPENING_ERROR if there was an error opening the CSV file.
 * - MEMORY_ALLOC_ERROR if there was an error allocating memory for the matrix.
 *
 * Note:
 * - The csv_matrix_ref parameter should be a triple pointer to char.
 * - The numRows and numCols parameters should be pointers to uint32_t.
 * - The delimiter parameter should be a pointer to char.
 *
 * Example usage:
 * - int32_t numRows;
 * - int32_t numCols;
 * - char ***csv_matrix;
 * - char delimiter = ',';
 * - int result = read_csv_file("data.csv", &csv_matrix, &numRows, &numCols, &delimiter);
 * - if (result == OK) {
 * -     // CSV file was successfully read and matrix was populated
 * - } else {
 * -     // Error occurred while reading CSV file or allocating memory
 * - }
 */
int read_csv_file(const char *filename, char ****csv_matrix_ref, uint32_t *numRows, uint32_t *numCols, char *delimiter)
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
    fclose(file);
    return OK;
}

/**
 * Function: free_csv_matrix
 * ----------------------------------
 * This function frees the memory allocated for a matrix of strings.
 *
 * Parameters:
 * - matrix: The matrix of strings to free.
 * - numRows: The number of rows in the matrix.
 * - numCols: The number of columns in the matrix.
 *
 * Example usage:
 * - char ***matrix;
 * - int32_t numRows;
 * - int32_t numCols;
 * - free_csv_matrix(matrix, numRows, numCols);
 */
void free_csv_matrix(char ***matrix, int32_t numRows, int32_t numCols)
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

/**
 * Function: parse_and_sort_json_integer_array
 * ----------------------------------
 * This function parses a JSON array of integers and sorts it.
 *
 * Parameters:
 * - json: The JSON array of integers to parse and sort.
 * - result_array_ref: A reference to the array that will store the parsed and sorted integers.
 * - nb_element: A pointer to a variable that will store the number of elements in the array.
 *
 * Returns:
 * - OK if the JSON array was successfully parsed and sorted.
 * - JSON_PARSING_ERROR if there was an error parsing the JSON array.
 * - MEMORY_ALLOC_ERROR if there was an error allocating memory for the array.
 *
 * Note:
 * - The result_array_ref parameter should be a pointer to a pointer to uint32_t.
 * - The nb_element parameter should be a pointer to uint32_t.
 *
 * Example usage:
 * - uint32_t *result_array;
 * - uint32_t nb_element;
 * - int result = parse_and_sort_json_integer_array("[3, 1, 2]", &result_array, &nb_element);
 * - if (result == OK) {
 * -     // JSON array was successfully parsed and sorted
 * - } else {
 * -     // Error occurred while parsing JSON array or allocating memory
 * - }
 */
int parse_and_sort_json_integer_array(const char *json, unsigned int **result_array_ref, unsigned int *nb_element){
    int ret = parse_json_integer_array(json, result_array_ref,nb_element);
    if(ret!=OK){
        return ret;
    }
    quick_sort(*result_array_ref, 0, (*nb_element)-1);
    return OK;
}

/**
 * Function: parse_json_integer_array
 * ----------------------------------
 * This function parses a JSON array of integers.
 *
 * Parameters:
 * - json: The JSON array of integers to parse.
 * - result_array_ref: A reference to the array that will store the parsed integers.
 * - nb_element: A pointer to a variable that will store the number of elements in the array.
 *
 * Returns:
 * - OK if the JSON array was successfully parsed.
 * - JSON_PARSING_ERROR if there was an error parsing the JSON array.
 * - MEMORY_ALLOC_ERROR if there was an error allocating memory for the array.
 *
 * Note:
 * - The result_array_ref parameter should be a pointer to a pointer to uint32_t.
 * - The nb_element parameter should be a pointer to uint32_t.
 *
 * Example usage:
 * - uint32_t *result_array;
 * - uint32_t nb_element;
 * - int result = parse_json_integer_array("[3, 1, 2]", &result_array, &nb_element);
 * - if (result == OK) {
 * -     // JSON array was successfully parsed
 * - } else {
 * -     // Error occurred while parsing JSON array or allocating memory
 * - }
 */
int parse_json_integer_array(const char *json, unsigned int **result_array_ref, unsigned int *nb_element)
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

/**
 * Function: add_double_unsigned_list_t
 * ----------------------------------
 * This function adds a new element to a list of double_unsigned_list_t structures.
 *
 * Parameters:
 * - head: A reference to the head of the list.
 * - u_value: The unsigned integer value to add to the list.
 * - d_value: The long double value to add to the list.
 *
 * Returns:
 * - OK if the element was successfully added to the list.
 * - MEMORY_ALLOC_ERROR if there was an error allocating memory for the new element.
 *
 * Note:
 * - The head parameter should be a pointer to a pointer to double_unsigned_list_t.
 *
 * Example usage:
 * - double_unsigned_list_t *head;
 * - int result = add_double_unsigned_list_t(&head, 1, 10.5);
 * - if (result == OK) {
 * -     // Element was successfully added to the list
 * - } else {
 * -     // Error occurred while allocating memory for the new element
 * - }
 */
int add_double_unsigned_list_t(double_unsigned_list_t **head, unsigned u_value, long double d_value)
{
    double_unsigned_list_t *new_list_t = (double_unsigned_list_t *)calloc(1, sizeof(double_unsigned_list_t));
    if (new_list_t == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return MEMORY_ALLOC_ERROR;
    }

    new_list_t->u_value = u_value;
    new_list_t->d_value = d_value;
    new_list_t->next = *head;
    *head = new_list_t;
    return OK;
}

/**
 * Function: free_double_unsigned_list_t
 * ----------------------------------
 * This function frees the memory allocated for a list of double_unsigned_list_t structures.
 *
 * Parameters:
 * - head: A reference to the head of the list.
 *
 * Example usage:
 * - double_unsigned_list_t *head;
 * - free_double_unsigned_list_t(head);
 */
void free_double_unsigned_list_t(double_unsigned_list_t *head){
    double_unsigned_list_t *current = head;
    double_unsigned_list_t *next_double_unsigned_node;

    while (current != NULL)
    {
        next_double_unsigned_node = current->next;
        free(current);
        current = next_double_unsigned_node;
    }
}

/**
 * Function: add_unsigned_list_t
 * ----------------------------------
 * This function adds a new element to a list of unsigned_list_t structures.
 *
 * Parameters:
 * - head: A reference to the head of the list.
 * - u_value: The unsigned integer value to add to the list.
 *
 * Returns:
 * - OK if the element was successfully added to the list.
 * - MEMORY_ALLOC_ERROR if there was an error allocating memory for the new element.
 *
 * Note:
 * - The head parameter should be a pointer to a pointer to unsigned_list_t.
 *
 * Example usage:
 * - unsigned_list_t *head;
 * - int result = add_unsigned_list_t(&head, 1);
 * - if (result == OK) {
 * -     // Element was successfully added to the list
 * - } else {
 * -     // Error occurred while allocating memory for the new element
 * - }
 */
int add_unsigned_list_t(unsigned_list_t **head, unsigned u_value){
    unsigned_list_t *new_list_t = (unsigned_list_t *)calloc(1, sizeof(unsigned_list_t));
    if (new_list_t == NULL)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return MEMORY_ALLOC_ERROR;
    }
    new_list_t->u_value = u_value;
    new_list_t->next = *head;
    *head = new_list_t;
    return OK;
}

/**
 * Function: free_unsigned_list_t
 * ----------------------------------
 * This function frees the memory allocated for a list of unsigned_list_t structures.
 *
 * Parameters:
 * - head: A reference to the head of the list.
 *
 * Example usage:
 * - unsigned_list_t *head;
 * - free_unsigned_list_t(head);
 */
void free_unsigned_list_t(unsigned_list_t *head){
    unsigned_list_t *current = head;
    unsigned_list_t *next_unsigned_node;

    while (current != NULL)
    {
        next_unsigned_node = current->next;
        free(current);
        current = next_unsigned_node;
    }
}

/**
 * Function: print_double_unsigned_list_t
 * ----------------------------------
 * This function prints the contents of a list of double_unsigned_list_t structures.
 *
 * Parameters:
 * - head: A reference to the head of the list.
 *
 * Example usage:
 * - double_unsigned_list_t *head;
 * - print_double_unsigned_list_t(head);
 */
void print_double_unsigned_list_t(double_unsigned_list_t *head)
{
    double_unsigned_list_t *current = head;
    while (current != NULL)
    {
        printf("%d %Lf\n", current->u_value, current->d_value);
        current = current->next;
    }
}

/**
 * Function: print_unsigned_list_t
 * ----------------------------------
 * This function prints the contents of a list of unsigned_list_t structures.
 *
 * Parameters:
 * - head: A reference to the head of the list.
 *
 * Example usage:
 * - unsigned_list_t *head;
 * - print_unsigned_list_t(head);
 */
void delete_value_in_unsigned_list(unsigned_list_t **head, unsigned u_value){
        unsigned_list_t *temp, *old;
        temp = *head;
        old = NULL;
        while (temp != NULL && temp->u_value != u_value)
        {
            old = temp;
            temp = temp->next;
        }
        if (old != NULL)
        {
            old->next = temp->next;
        }
        else
        {
            *head = temp->next;
        }
        free(temp);
}


/*
 * Function: swap
 * ----------------------------------
 * This function swaps the values of two unsigned integers.
 *
 * Parameters: 
 * - a: pointer to the first unsigned integer
 * - b: pointer to the second unsigned integer
 *
 * Returns: 
 * - None
 *
 * Note: 
 * The function modifies the values of the input pointers before returning.
 *
 * Example usage: 
 * - swap(&num1, &num2);
 */
void swap(unsigned int *a, unsigned int *b) {
    unsigned int temp = *a;
    *a = *b;
    *b = temp;
}

/*
 * Function: partition
 * ----------------------------------
 * This function partitions an array by selecting a pivot element and rearranging the elements such that all elements smaller than the pivot are placed before it, and all elements greater than the pivot are placed after it.
 *
 * Parameters:
 * - arr: The array to be partitioned.
 * - low: The starting index of the partition.
 * - high: The ending index of the partition.
 *
 * Returns:
 * - The index of the pivot element after partitioning.
 *
 * Note:
 * - The function uses the last element of the array as the pivot.
 * - The function modifies the original array.
 *
 * Example usage:
 * - int arr[] = {5, 2, 8, 1, 9};
 *   int n = sizeof(arr) / sizeof(arr[0]);
 *   int pivotIndex = partition(arr, 0, n - 1);
 */
int partition(unsigned int arr[], int low, int high) {
    unsigned int pivot = arr[high];
    int i = (low - 1);
    
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

/*
 * Function: quick_sort
 * ----------------------------------
 * This function implements the quicksort algorithm to sort an array of unsigned integers in ascending order.
 *
 * Parameters:
 * - arr: The array to be sorted.
 * - low: The starting index of the subarray to be sorted.
 * - high: The ending index of the subarray to be sorted.
 *
 * Returns:
 * - None. The array is sorted in-place.
 *
 * Note:
 * - This function uses the partition function to divide the array into two subarrays and recursively sorts them.
 * - The pivot element is chosen as the last element of the subarray.
 * - The sorting is done in ascending order.
 *
 * Example usage:
 * - unsigned int arr[] = {5, 2, 8, 1, 9};
 *   quick_sort(arr, 0, 4);
 *   // The array arr is now {1, 2, 5, 8, 9}
 */
void quick_sort(unsigned int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        
        quick_sort(arr, low, pi - 1);
        quick_sort(arr, pi + 1, high);
    }
}

/*
 * Function: value_is_in_array
 * ----------------------------------
 * This function performs a binary search on a sorted array to find a target value.
 *
 * Parameters:
 * - arr: A pointer to the sorted array.
 * - size: The size of the array.
 * - target: The value to search for.
 *
 * Returns:
 * - true if the target value is found in the array, false otherwise.
 *
 * Note:
 * - The array must be sorted in ascending order.
 * - The function assumes that the array is not empty.
 * - The function assumes that the size parameter is greater than 0.
 * - The function uses the binary search algorithm to find the target value.
 * - The function returns true if the target value is found, false otherwise.
 *
 * Example usage:
 * - bool found = value_is_in_array(arr, size, target);
 */
bool value_is_in_array(uint32_t *arr, uint32_t size, uint32_t target) {
    uint32_t low = 0;
    uint32_t high = size - 1;

    while (low <= high) {
        uint32_t mid = low + ((high - low) / 2);
        if(mid >=size){
            return false;
        }
        if (arr[mid] == target) {
            return true;
        } else if (arr[mid] < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return false;
}