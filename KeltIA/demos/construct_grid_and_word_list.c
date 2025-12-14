// demos/construct_grid_and_word_list.c

#include "../include/nn/grid_builder.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <image> <model>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("╔════════════════════════════════════════╗\n");
    printf("║  Construct Grid and Word List         ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    char **words;
    int num_words;
    int error;

    // ONE FUNCTION CALL!
    GridResult grid =
        process_crossword_image(argv[1], argv[2], &words, &num_words, &error);

    if (error != CROSSWORD_OK)
    {
        fprintf(stderr, "Error: %d\n", error);
        return EXIT_FAILURE;
    }

    // Print grid
    printf("╔════════════════════════════════════════╗\n");
    printf("║              GRID                     ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    for (int i = 0; i < grid.height; i++)
    {
        printf("  ");
        for (int j = 0; j < grid.width; j++)
        {
            printf("%c", grid.char_grid[i * grid.width + j]);
        }
        printf("\n");
    }

    // Print words
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║            WORD LIST                  ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    for (int i = 0; words[i] != NULL; i++)
    {
        printf("  %2d. %s\n", i + 1, words[i]);
    }

    printf("\n");

    // Cleanup
    free_crossword_result(&grid, words);

    printf("✓ Done!\n");
    return EXIT_SUCCESS;
}
