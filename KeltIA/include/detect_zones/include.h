#pragma once

typedef struct
{
    int r1, c1;  // Top-left corner
    int r2, c2;  // Bottom-right corner
} Rectangle;

int **allocate_matrix(int rows, int cols);
void free_matrix(int **matrix, int rows);
int **partition(int **image, int rows, int cols, int n, double threshold);
void dfs(
    int **matrix,
    int **visited,
    int rows,
    int cols,
    int x,
    int y,
    Rectangle *rect
);

Rectangle *find_rectangles(int **matrix, int rows, int cols, int *count);
void detect_zones(int **image, int rows, int cols, int n, double threshold);
int **read_matrix_file(const char *filename, int *rows, int *cols);

void print_matrix_emoji(int **M, int rows, int cols);

void one_function_to_rule_them_all(
    int **matrix,
    int width,
    int height,
    int n,
    double threshold
);
