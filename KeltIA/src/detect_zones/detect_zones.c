#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/detect_zones/include.h"

int **allocate_matrix(int rows, int cols)
{
    int **matrix = malloc(rows * sizeof(int *));
    for (int i = 0; i < rows; i++)
    {
        matrix[i] = calloc(cols, sizeof(int));  // calloc initializes to 0
    }
    return matrix;
}

void free_matrix(int **matrix, int rows)
{
    for (int i = 0; i < rows; i++) free(matrix[i]);
    free(matrix);
}

int **partition(int **image, int rows, int cols, int n, double threshold)
{
    int **grid = allocate_matrix(n, n);

    int block_h = rows / n;
    int block_w = cols / n;

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            // Calculate block boundaries
            int start_row = i * block_h + (i < rows % n ? i : rows % n);
            int end_row = start_row + block_h + (i < rows % n ? 1 : 0);
            int start_col = j * block_w + (j < cols % n ? j : cols % n);
            int end_col = start_col + block_w + (j < cols % n ? 1 : 0);

            // Count black pixels
            int black_pixels = 0, total_pixels = 0;
            for (int r = start_row; r < end_row && r < rows; r++)
            {
                for (int c = start_col; c < end_col && c < cols; c++)
                {
                    black_pixels += image[r][c];
                    total_pixels++;
                }
            }

            grid[i][j] =
                ((double)black_pixels / total_pixels > threshold) ? 1 : 0;
        }
    }

    return grid;
}

// ============= DFS AND RECTANGLE SEARCH =============
void dfs(
    int **matrix,
    int **visited,
    int rows,
    int cols,
    int x,
    int y,
    Rectangle *rect
)
{
    if (x < 0 || x >= rows || y < 0 || y >= cols || visited[x][y] ||
        matrix[x][y] != 1)
        return;

    visited[x][y] = 1;

    // Update rectangle boundaries
    if (x < rect->r1) rect->r1 = x;
    if (x > rect->r2) rect->r2 = x;
    if (y < rect->c1) rect->c1 = y;
    if (y > rect->c2) rect->c2 = y;

    // Explore neighbors (4 directions)
    dfs(matrix, visited, rows, cols, x - 1, y, rect);
    dfs(matrix, visited, rows, cols, x + 1, y, rect);
    dfs(matrix, visited, rows, cols, x, y - 1, rect);
    dfs(matrix, visited, rows, cols, x, y + 1, rect);
}

Rectangle *find_rectangles(int **matrix, int rows, int cols, int *count)
{
    int **visited = allocate_matrix(rows, cols);
    Rectangle *rectangles = malloc(rows * cols * sizeof(Rectangle));
    *count = 0;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (matrix[i][j] == 1 && !visited[i][j])
            {
                rectangles[*count] = (Rectangle){i, j, i, j};
                dfs(matrix, visited, rows, cols, i, j, &rectangles[*count]);
                (*count)++;
            }
        }
    }

    free_matrix(visited, rows);
    return rectangles;
}

void detect_zones(int **image, int rows, int cols, int n, double threshold)
{
    // Partition image into grid
    int **grid = partition(image, rows, cols, n, threshold);

    // Find rectangles in grid
    int rect_count;
    Rectangle *rectangles = find_rectangles(grid, n, n, &rect_count);

    if (rect_count < 2)
    {
        fprintf(stderr, "Error: Found less than 2 rectangles\n");
        free_matrix(grid, n);
        free(rectangles);
        exit(1);
    }

    // Identify grid and word list by area
    int area1 = (rectangles[0].r2 - rectangles[0].r1 + 1) *
                (rectangles[0].c2 - rectangles[0].c1 + 1);
    int area2 = (rectangles[1].r2 - rectangles[1].r1 + 1) *
                (rectangles[1].c2 - rectangles[1].c1 + 1);

    Rectangle *grid_rect = (area1 > area2) ? &rectangles[0] : &rectangles[1];
    Rectangle *words_rect = (area1 > area2) ? &rectangles[1] : &rectangles[0];

    // Convert to original coordinates
    int scale_h = rows / n;
    int scale_w = cols / n;

    printf(
        "Grid: ((%d, %d), (%d, %d))\n", grid_rect->r1 * scale_h,
        grid_rect->c1 * scale_w, (grid_rect->r2 + 1) * scale_h,
        (grid_rect->c2 + 1) * scale_w
    );

    printf(
        "Words: ((%d, %d), (%d, %d))\n", words_rect->r1 * scale_h,
        words_rect->c1 * scale_w, (words_rect->r2 + 1) * scale_h,
        (words_rect->c2 + 1) * scale_w
    );

    free_matrix(grid, n);
    free(rectangles);
}

int **read_matrix_file(const char *filename, int *rows, int *cols)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Error: Cannot open %s\n", filename);
        exit(1);
    }

    char line[2048];
    int **matrix = malloc(1000 * sizeof(int *));
    *rows = 0;
    *cols = 0;

    while (fgets(line, sizeof(line), file))
    {
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[--len] = '\0';
        if (len == 0) continue;

        matrix[*rows] = malloc(len * sizeof(int));
        for (int i = 0; i < len; i++)
        {
            matrix[*rows][i] = (line[i] == '1') ? 1 : 0;
        }

        if (*cols == 0) *cols = len;
        (*rows)++;
    }

    fclose(file);
    return matrix;
}

// ============= MATRIX PRINTING FUNCTIONS =============

/**
 * Prints a matrix in numeric format
 * @param M: Matrix to print
 * @param rows: Number of rows
 * @param cols: Number of columns
 */
void print_matrix(int **M, int rows, int cols)
{
    printf("=========== Matrix ===========\n");
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++) { printf("%d  ", M[i][j]); }
        printf("\n");
    }
    printf("==============================\n");
}

#define RESET "\033[0m"
#define BLACK "\033[40m  \033[0m"         // Black background
#define WHITE "\033[47m  \033[0m"         // White background
#define BLUE "\033[44m  \033[0m"          // Blue background
#define GREEN "\033[42m  \033[0m"         // Green background
#define RED "\033[41m  \033[0m"           // Red background
#define ORANGE "\033[48;5;208m  \033[0m"  // Orange background (256 colors)

const char *convert_to_color(int val)
{
    switch (val)
    {
    case 0:
        return BLACK;  // Black block
    case 1:
        return WHITE;  // White block
    case 2:
        return BLUE;  // Blue block
    case 3:
        return GREEN;  // Green block
    case 4:
        return RED;  // Red block
    case 5:
        return ORANGE;  // Orange block
    default:
        return "\033[45m??\033[0m";  // Magenta with '??' for unknown
    }
}

/**
 * Prints a matrix using emoji visualization
 * @param M: Matrix to print
 * @param rows: Number of rows
 * @param cols: Number of columns
 */
void print_matrix_emoji(int **M, int rows, int cols)
{
    printf("============ Matrix ============\n");
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%s ", convert_to_color(M[i][j]));
        }
        printf("\n");
    }
    printf("================================\n");
}

/*
 * This function tests three stages:
 *   1. Partition test. Displays the resulting matrix
 *   2. Find Rectangles test. Displays graphically and numerically
 *   3. Main function test, numerically
 */
void one_function_to_rule_them_all(
    int **matrix,
    int width,
    int height,
    int n,
    double threshold
)
{
    printf("▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬ STAGE 1 ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬\n");

    // Create the small partitioned matrix
    int **smallerMatrix = partition(matrix, width, height, n, threshold);
    print_matrix(smallerMatrix, n, n);

    printf(
        "\n▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬ STAGE 2 ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬\n"
    );
    int rect_count;
    Rectangle *rectangles = find_rectangles(smallerMatrix, n, n, &rect_count);

    if (rect_count < 2)
    {
        fprintf(stderr, "Not enough rectangles found.\n");

        free(rectangles);
        free_matrix(smallerMatrix, n);
        free_matrix(matrix, height);

        exit(1);
    }

    Rectangle rectangleA = rectangles[0];
    Rectangle rectangleB = rectangles[1];

    // Mark corners with colors
    smallerMatrix[rectangleA.r1][rectangleA.c1] = 2;  // Blue
    smallerMatrix[rectangleA.r2][rectangleA.c2] = 3;  // Green
    smallerMatrix[rectangleB.r1][rectangleB.c1] = 4;  // Red
    smallerMatrix[rectangleB.r2][rectangleB.c2] = 5;  // Orange

    print_matrix_emoji(smallerMatrix, n, n);

    int areaA =
        (rectangleA.r2 - rectangleA.r1) * (rectangleA.c2 - rectangleA.c1);
    int areaB =
        (rectangleB.r2 - rectangleB.r1) * (rectangleB.c2 - rectangleB.c1);
    Rectangle grid, words;
    if (areaA > areaB)
    {
        grid = rectangleA;
        words = rectangleB;
    }
    else
    {
        grid = rectangleB;
        words = rectangleA;
    }

    // Blue and green
    printf(
        "\033[34m██ \033[0m\033[32m██\033[0m [grid] Grid coordinates are "
        "((%d,%d),(%d,%d))\n",
        grid.r1, grid.c1, grid.r2, grid.c2
    );

    // Red and orange
    printf(
        "\033[31m██ \033[0m\033[38;5;208m██\033[0m [words] The word list "
        "coordinates are ((%d,%d),(%d,%d))\n",
        words.r1, words.c1, words.r2, words.c2
    );

    printf(
        "\n▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬ STAGE 3 ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬\n"
    );
    detect_zones(matrix, width, height, n, threshold);

    free_matrix(smallerMatrix, n);
    free(rectangles);
}
