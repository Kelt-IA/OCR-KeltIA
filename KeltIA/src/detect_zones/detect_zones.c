#include <stdio.h>
#include <stdlib.h>

/*
MatrixPicture : Contains only 0 (white) or 1 (black) values.
n : Number of rows and columns there should be in the grid.
seuil : Real number in [0, 1]. Determines the level the square is considered white or black.
*/
int** partition(int** MatrixPicture, int num_rows, int num_cols, int n, double seuil) {
    if (n > num_rows || n > num_cols) {
        fprintf(stderr, "Erreur : n est trop grand.\n");
        exit(1);
    }

    // grid : n*n matrix initially filled with -1.
    int** grid = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        grid[i] = (int*)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++) {
            grid[i][j] = -1;
        }
    }

    // block : the width or height of the rows or columns corresponding to a single grid cell.
    int block_h = num_rows / n;
    int reste_h = num_rows % n;
    int block_w = num_cols / n;
    int reste_w = num_cols % n;

    int x = 0;
    for (int a = 0; a < n; a++) {
        int h = block_h + (a < reste_h);
        int a2;  // <-- Déclaration corrigée ici
        int y = 0;
        for (int b = 0; b < n; b++) {
            int w = block_w + (b < reste_w);

            int b2 = (y + w < num_cols) ? y + w : num_cols;
            a2 = (x + h < num_rows) ? x + h : num_rows;

            // Checks if current square contains more black pixels than white according to the seuil.
            int total = 0;
            int nb = 0;
            for (int i = x; i < a2; i++) {
                for (int j = y; j < b2; j++) {
                    total += MatrixPicture[i][j];
                    nb += 1;
                }
            }

            double moyenne = (double)total / nb;
            int contains_word = (moyenne > seuil) ? 1 : 0;
            grid[a][b] = contains_word;

            y = b2;  // avancer horizontalement
        }
        x = a2;  // avancer verticalement
    }

    return grid;
}

/*
Takes as parameter a matrix.
Returns the location of the two main rectangles.
We advise to apply partition() before applying find_rectangles().
*/
typedef struct {
    int r1, c1;
    int r2, c2;
} Rectangle;

Rectangle* find_rectangles(int** matrix, int rows, int cols, int* rect_count) {
    int** visited = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        visited[i] = (int*)calloc(cols, sizeof(int));
    }

    Rectangle* rectangles = (Rectangle*)malloc(rows * cols * sizeof(Rectangle));
    int rect_idx = 0;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (matrix[i][j] == 1 && visited[i][j] == 0) {
                // Stack for DFS
                int stack_size = rows * cols;
                int (*stack)[2] = malloc(stack_size * sizeof *stack);
                int top = 0;
                stack[top][0] = i;
                stack[top][1] = j;
                top++;

                int min_r = i, max_r = i, min_c = j, max_c = j;

                while (top > 0) {
                    top--;
                    int x = stack[top][0];
                    int y = stack[top][1];
                    if (visited[x][y] == 1) continue;
                    visited[x][y] = 1;

                    if (x < min_r) min_r = x;
                    if (x > max_r) max_r = x;
                    if (y < min_c) min_c = y;
                    if (y > max_c) max_c = y;

                    // Neighbours
                    if (x > 0 && matrix[x - 1][y] == 1 && visited[x - 1][y] == 0) {
                        stack[top][0] = x - 1; stack[top][1] = y; top++;
                    }
                    if (x < rows - 1 && matrix[x + 1][y] == 1 && visited[x + 1][y] == 0) {
                        stack[top][0] = x + 1; stack[top][1] = y; top++;
                    }
                    if (y > 0 && matrix[x][y - 1] == 1 && visited[x][y - 1] == 0) {
                        stack[top][0] = x; stack[top][1] = y - 1; top++;
                    }
                    if (y < cols - 1 && matrix[x][y + 1] == 1 && visited[x][y + 1] == 0) {
                        stack[top][0] = x; stack[top][1] = y + 1; top++;
                    }
                }

                rectangles[rect_idx].r1 = min_r;
                rectangles[rect_idx].c1 = min_c;
                rectangles[rect_idx].r2 = max_r;
                rectangles[rect_idx].c2 = max_c;
                rect_idx++;

                free(stack);
            }
        }
    }

    *rect_count = rect_idx;
    return rectangles;
}

/*
Takes 3 parameters and returns the coordinates of the grid, then the coordinates of the words list
Parameters :
    - matrix : A matrix of booleans (0 or 1). (0 is white, 1 is black). We are looking for "1" pixels.
    - n : number of subdivisions. The grid will be a n*n matrix.
    - seuil : tolerance for 1 pixels proportion per grid square to consider it "1".
*/
void main_function(int** matrix, int width, int height, int n, double seuil) {
    int w = width / n;
    int h = height / n;

    int** smallerMatrix = partition(matrix, width, height, n, seuil);

    int rect_count;
    Rectangle* rectangles = find_rectangles(smallerMatrix, n, n, &rect_count);

    if (rect_count < 2) {
        fprintf(stderr, "Not enough rectangles found. Check threshold or input matrix.\n");
        exit(1);
    }

    Rectangle rectangleA = rectangles[0];
    Rectangle rectangleB = rectangles[1];
    int areaA = (rectangleA.r2 - rectangleA.r1) * (rectangleA.c2 - rectangleA.c1);
    int areaB = (rectangleB.r2 - rectangleB.r1) * (rectangleB.c2 - rectangleB.c1);

    Rectangle grid, words;
    if (areaA > areaB) {
        grid = rectangleA;
        words = rectangleB;
    } else {
        grid = rectangleB;
        words = rectangleA;
    }

    // Convert to original matrix coordinates
    int grid_r1 = grid.r1 * w;
    int grid_c1 = grid.c1 * h;
    int grid_r2 = grid.r2 * w;
    int grid_c2 = (grid.c2 + 1) * h;

    int words_r1 = words.r1 * w;
    int words_c1 = words.c1 * h;
    int words_r2 = words.r2 * w;
    int words_c2 = (words.c2 + 1) * h;

    printf("Grid: ((%d, %d), (%d, %d))\n", grid_r1, grid_c1, grid_r2, grid_c2);
    printf("Words: ((%d, %d), (%d, %d))\n", words_r1, words_c1, words_r2, words_c2);

    // Libération de la mémoire
    for (int i = 0; i < n; i++) free(smallerMatrix[i]);
    free(smallerMatrix);
    free(rectangles);
}
