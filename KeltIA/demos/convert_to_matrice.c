#include "../include/detect_zones/detect_char.h"
#include <err.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        errx(EXIT_FAILURE, "Usage: %s <input_image> <output_image>\n", argv[0]);
        errx(EXIT_FAILURE, "Extract characters from grid and wordlist.\n");
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];

    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    if (!MagickReadImage(wand, input_path))
    {
        DestroyMagickWand(wand);
        MagickWandTerminus();
        errx(EXIT_FAILURE, "Erreur: Could not read %s\n", input_path);
    }

    ExtractedZones ez = detect_zones(wand);

    // Detect characters in the grid
    int grid_chars = 0;
    CharBBox *grid_characters = detect_characters(
        wand, ez.grid.x_min, ez.grid.y_min, ez.grid.x_max - ez.grid.x_min,
        ez.grid.y_max - ez.grid.y_min, &grid_chars
    );

    // Detect words int the list
    int words_chars = 0;
    CharBBox *words_characters = detect_characters(
        wand, ez.words.x_min, ez.words.y_min, ez.words.x_max - ez.words.x_min,
        ez.words.y_max - ez.words.y_min, &words_chars
    );

    DrawingWand *draw = NewDrawingWand();
    DrawLetterBoundries(wand, draw, grid_characters, grid_chars, "blue");
    DrawLetterBoundries(wand, draw, words_characters, words_chars, "green");

    // Test build_matrix for grid characters
    if (grid_chars > 0)
    {
        int grid_num_rows = 0;
        int *grid_counts =
            count_chars_per_row(grid_characters, grid_chars, &grid_num_rows);
        if (grid_counts && grid_num_rows > 0)
        {
            int grid_length = grid_counts[0];  // Assume all rows have the same
                                               // number of columns
            CharBBox **grid_matrix =
                build_matrix(grid_characters, grid_chars, grid_length);
            if (grid_matrix)
            {
                printf(
                    "Grid matrix created with %d rows and %d columns per row\n",
                    grid_num_rows, grid_length
                );
                // Print the matrix
                printf("Grid matrix:\n");
                for (int r = 0; r < grid_num_rows; r++)
                {
                    printf("Row %d: ", r);
                    for (int c = 0; c < grid_length; c++)
                    {
                        if (grid_matrix[r][c].w > 0)
                        {  // Assuming w=0 means empty
                            printf(
                                "(%d,%d) ", grid_matrix[r][c].x,
                                grid_matrix[r][c].y
                            );
                        }
                        else { printf("(empty) "); }
                    }
                    printf("\n");
                }
                // Free the matrix
                for (int i = 0;
                     i < (grid_chars + grid_length - 1) / grid_length; i++)
                {
                    free(grid_matrix[i]);
                }
                free(grid_matrix);
            }
            free(grid_counts);
        }
    }

    // Test build_matrix for words characters
    if (words_chars > 0)
    {
        int words_num_rows = 0;
        int *words_counts =
            count_chars_per_row(words_characters, words_chars, &words_num_rows);
        if (words_counts && words_num_rows > 0)
        {
            int words_length = words_counts[0];  // Assume all rows have the
                                                 // same number of columns
            CharBBox **words_matrix =
                build_matrix(words_characters, words_chars, words_length);
            if (words_matrix)
            {
                printf(
                    "Words matrix created with %d rows and %d columns per "
                    "row\n",
                    words_num_rows, words_length
                );
                // Print the matrix
                printf("Words matrix:\n");
                for (int r = 0; r < words_num_rows; r++)
                {
                    printf("Row %d: ", r);
                    for (int c = 0; c < words_length; c++)
                    {
                        if (words_matrix[r][c].w > 0)
                        {  // Assuming w=0 means empty
                            printf(
                                "(%d,%d) ", words_matrix[r][c].x,
                                words_matrix[r][c].y
                            );
                        }
                        else { printf("(empty) "); }
                    }
                    printf("\n");
                }
                // Free the matrix
                for (int i = 0;
                     i < (words_chars + words_length - 1) / words_length; i++)
                {
                    free(words_matrix[i]);
                }
                free(words_matrix);
            }
            free(words_counts);
        }
    }

    free(grid_characters);
    free(words_characters);

    MagickDrawImage(wand, draw);

    MagickWriteImage(wand, output_path);
    DestroyDrawingWand(draw);

    DestroyMagickWand(wand);
    MagickWandTerminus();
}
