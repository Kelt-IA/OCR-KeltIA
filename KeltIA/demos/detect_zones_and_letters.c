#include "../include/detect_zones/detect_char.h"
#include "../include/detect_zones/detect_zones.h"
#include <err.h>
#include <string.h>
#include <sys/stat.h>

const char *extract_filename(const char *path)
{
    if (path == NULL) { return NULL; }

    const char *last_slash = strrchr(path, '/');

    // If no slash found, return the entire string
    if (last_slash == NULL) { return path; }

    // Return everything after the last slash
    return last_slash + 1;
}

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4)
    {
        errx(
            EXIT_FAILURE,
            "Usage: %s <input_image> <output_image> [--save]\n"
            "  --save: saves extracted characters to disk\n",
            argv[0]
        );
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];

    char file_prefix[256];
    const char *filename = extract_filename(input_path);
    strncpy(file_prefix, filename, sizeof(file_prefix) - 1);
    file_prefix[sizeof(file_prefix) - 1] = '\0';

    // Remove extension
    char *dot = strrchr(file_prefix, '.');
    if (dot) { *dot = '\0'; }

    // Check for --save flag
    int save_characters = 0;
    if (argc == 4 && strcmp(argv[3], "--save") == 0) { save_characters = 1; }

    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    if (!MagickReadImage(wand, input_path))
    {
        DestroyMagickWand(wand);
        MagickWandTerminus();
        errx(EXIT_FAILURE, "Error: Could not read %s\n", input_path);
    }

    ExtractedZones ez = detect_zones(wand);

    // Detect characters in the grid
    int grid_chars = 0;
    CharBBox *grid_characters = detect_characters(
        wand, ez.grid.x_min, ez.grid.y_min, ez.grid.x_max - ez.grid.x_min,
        ez.grid.y_max - ez.grid.y_min, &grid_chars
    );

    // Detect words in the list
    int words_chars = 0;
    CharBBox *words_characters = detect_characters(
        wand, ez.words.x_min, ez.words.y_min, ez.words.x_max - ez.words.x_min,
        ez.words.y_max - ez.words.y_min, &words_chars
    );

    printf("Letters detected in grid: %d\n", grid_chars);
    printf("Letters detected in wordlist: %d\n", words_chars);

    // Save extracted characters if --save flag is present
    if (save_characters)
    {
        printf("\n=== Saving Extracted Characters ===\n");

        // Create output directories
        const char *output_dir = "extracted_characters";
        mkdir(output_dir, 0755);

        const char *output_dir_grid = "extracted_characters/grid";
        mkdir(output_dir_grid, 0755);

        const char *output_dir_list = "extracted_characters/list";
        mkdir(output_dir_list, 0755);

        // Save all characters from grid
        printf("\nGrid characters (%d found):\n", grid_chars);
        int saved_grid = 0;
        for (int i = 0; i < grid_chars; i++)
        {
            char filepath[512];
            snprintf(
                filepath, sizeof(filepath), "%s/%s_grid_char_%03d.bmp",
                output_dir_grid, file_prefix, i
            );

            int result =
                save_charbbox_as_bitmap(wand, grid_characters[i], filepath);
            if (result == 0)
            {
                saved_grid++;
                if (saved_grid <= 5 || saved_grid == grid_chars)
                {
                    printf(
                        "  ✓ [%d] %s (pos: %d,%d size: %dx%d)\n", i, filepath,
                        grid_characters[i].x, grid_characters[i].y,
                        grid_characters[i].w, grid_characters[i].h
                    );
                }
                else if (saved_grid == 6)
                {
                    printf("  ... (saving remaining characters) ...\n");
                }
            }
            else
            {
                printf("  ✗ [%d] Failed to save %s\n", i, filepath);
            }
        }
        printf("Total grid characters saved: %d/%d\n", saved_grid, grid_chars);

        // Save all characters from wordlist
        printf("\nWordlist characters (%d found):\n", words_chars);
        int saved_words = 0;
        for (int i = 0; i < words_chars; i++)
        {

            char filepath[512];
            snprintf(
                filepath, sizeof(filepath), "%s/%s_word_char_%03d.bmp",
                output_dir_list, file_prefix, i
            );

            int result =
                save_charbbox_as_bitmap(wand, words_characters[i], filepath);
            if (result == 0)
            {
                saved_words++;
                if (saved_words <= 5 || saved_words == words_chars)
                {
                    printf(
                        "  ✓ [%d] %s (pos: %d,%d size: %dx%d)\n", i, filepath,
                        words_characters[i].x, words_characters[i].y,
                        words_characters[i].w, words_characters[i].h
                    );
                }
                else if (saved_words == 6)
                {
                    printf("  ... (saving remaining characters) ...\n");
                }
            }
            else
            {
                printf("  ✗ [%d] Failed to save %s\n", i, filepath);
            }
        }
        printf(
            "Total wordlist characters saved: %d/%d\n", saved_words, words_chars
        );
    }

    // Draw bounding boxes on output image
    DrawingWand *draw = NewDrawingWand();

    // Draw letters
    DrawLetterBoundries(wand, draw, grid_characters, grid_chars, "blue");
    DrawLetterBoundries(wand, draw, words_characters, words_chars, "yellow");

    // Draw zones
    DrawZoneBoundries(draw, &ez.grid, "red");
    DrawZoneBoundries(draw, &ez.words, "green");

    printf(
        "\nGrid zone: x1:%i y1:%i x2:%i y2:%i\n", ez.grid.x_min, ez.grid.y_min,
        ez.grid.x_max, ez.grid.y_max
    );
    printf(
        "Wordlist zone: x1:%i y1:%i x2:%i y2:%i\n", ez.words.x_min,
        ez.words.y_min, ez.words.x_max, ez.words.y_max
    );

    // Cleanup
    free(grid_characters);
    free(words_characters);

    MagickDrawImage(wand, draw);
    MagickWriteImage(wand, output_path);

    DestroyDrawingWand(draw);
    DestroyMagickWand(wand);
    MagickWandTerminus();

    printf("\nOutput image saved to: %s\n", output_path);

    return EXIT_SUCCESS;
}
