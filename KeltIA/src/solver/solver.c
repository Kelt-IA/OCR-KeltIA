#include "../../include/solver/include_solver.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
The main function is solver(grid, word).

Algorithm :
    for each (i,j) case in the grid :
        for each of the 8 directions, search the word :
            - if the word was found :
                return coordinates (startX,startY),(endX,endY)
    return None
*/

void solver(char *grid, int height, int width, const char *word, int word_len)
{
    int directions[8][2] = {
        {0, 1},    // → E  (East)
        {0, -1},   // ← W  (West)
        {1, 0},    // ↓ S  (South)
        {-1, 0},   // ↑ N  (North)
        {1, 1},    // ↘ SE (South-East)
        {-1, -1},  // ↖ NW (North-West)
        {1, -1},   // ↙ SW (South-West)
        {-1, 1}    // ↗ NE (North-East)
    };

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            if (grid[i * width + j] ==
                word[0])  // grid[i][j] => grid[i*width+j]
            {
                for (int d = 0; d < 8; d++)
                {
                    int abscisse = directions[d][0];
                    int ordonnee = directions[d][1];

                    int x = i;
                    int y = j;
                    int k = 0;

                    while (k < word_len && x >= 0 && x < height && y >= 0 &&
                           y < width && grid[x * width + y] == word[k])
                    {
                        x += abscisse;
                        y += ordonnee;
                        k++;
                    }

                    if (k == word_len)
                    {
                        int i2 = x - abscisse;
                        int j2 = y - ordonnee;

                        printf(
                            "(%i,%i)(%i,%i)\n", j, i, j2, i2
                        );  // Python and C's indexes (i,j) are (j,i) humans
                            // coordinates.
                        return;
                    }
                }
            }
        }
    }
    printf("Not Found\n");
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
        errx(1, "Missing arguments. Expected : ./solver [file] [word]");

    char *file = argv[1];
    char *word = argv[2];

    int word_len = strlen(word);

    // Capitalize the Word
    for (int i = 0; i < word_len; i++)
        if ('a' <= word[i] && word[i] <= 'z') word[i] -= 32;  // 32 = 'a'-'A';

    // TODO : File as parameter.
    int height;
    int width;

    char **grid;
    grid = readFile(file, &height, &width);
    if (grid == NULL)
    {
        printf("There was an error reading the file provided\n");
        free_grid(grid);
        exit(1);
    }

    // // grid
    // int height = 9;
    // int width = 10;
    // char *grid = malloc(height * width * sizeof(char));
    // char data[9][11] = {"HORIZONTAL", "DXRAHCLBGA", "DIKCILEOKC",
    //                     "IGAJHYLYHI", "HGFGODTIOT", "GDLROWKBFR",
    //                     "PLNRDNERGE", "JHAIDUAJGV", "UKGFFOLLEH"};
    // for (int i = 0; i < height; i++)
    //     for (int j = 0; j < width; j++) grid[i * width + j] = data[i][j];

    solver(*grid, height, width, word, word_len);

    free_grid(grid);
    return 0;
}
