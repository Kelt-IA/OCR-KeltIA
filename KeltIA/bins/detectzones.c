#include "../include/detect_zones/include.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: %s <file> <n> <threshold>\n", argv[0]);
        return 1;
    }

    int rows, cols;
    int **matrix = read_matrix_file(argv[1], &rows, &cols);
    int n = atoi(argv[2]);
    double threshold = atof(argv[3]);

    // Use the complete testing function instead
    one_function_to_rule_them_all(matrix, rows, cols, n, threshold);

    free_matrix(matrix, rows);
    return 0;
}
