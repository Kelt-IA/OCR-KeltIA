#include "../../include/solver/include_solver.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    solver(*grid, height, width, word, word_len);

    free_grid(grid);
    return 0;
}
