#include "../include/detect_zones/detect_letters.h"
#include "../include/detect_zones/detect_zones.h"
#include <err.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        errx(EXIT_FAILURE, "Usage: %s <input_image> <output_image>\n", argv[0]);
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
    CharBBox *grid_characters = detect_letters(
        wand, ez.grid.x_min, ez.grid.y_min, ez.grid.x_max - ez.grid.x_min,
        ez.grid.y_max - ez.grid.y_min, &grid_chars
    );

    // Detect words int the list
    int words_chars = 0;
    CharBBox *words_characters = detect_letters(
        wand, ez.words.x_min, ez.words.y_min, ez.words.x_max - ez.words.x_min,
        ez.words.y_max - ez.words.y_min, &words_chars
    );

    printf("letters detected in grid: %d\n", grid_chars);
    printf("letters detected in wordlist: %d\n", words_chars);

    // MagickDrawImage(wand, draw);

    /*
    // Create output directory for extracted characters
    const char *output_dir = "extracted_characters";
    mkdir(output_dir, 0755);

    const char *output_dir_grid = "extracted_characters/grid";
    mkdir(output_dir_grid, 0755);

    const char *output_dir_list = "extracted_characters/list";
    mkdir(output_dir_list, 0755);

    // Save all characters from grid
    printf("\n=== Saving extracted characters ===\n");
    printf("Grid characters (%d found):\n", grid_chars);
    for (int i = 0; i < grid_chars; i++)
    {
        char filepath[512];
        snprintf(
            filepath, sizeof(filepath), "%s/grid_char_%03d.bmp",
            output_dir_grid, i
        );

        int result =
            save_charbbox_as_bitmap(wand, grid_characters[i], filepath);
        if (result == 0)
        {
            printf(
                "  ✓ [%d] %s (pos: %d,%d size: %dx%d)\n", i, filepath,
                grid_characters[i].x, grid_characters[i].y,
                grid_characters[i].w, grid_characters[i].h
            );
        }
        else
        {
            printf("  ✗ [%d] Failed to save %s\n", i, filepath);
        }
    }


    // Save all characters from wordlist
    printf("\nWordlist characters (%d found):\n", words_chars);
    for (int i = 0; i < words_chars; i++)
    {
        char filepath[512];
        snprintf(
            filepath, sizeof(filepath), "%s/word_char_%03d.bmp",
            output_dir_list, i
        );

        int result =
            save_charbbox_as_bitmap(wand, words_characters[i], filepath);
        if (result == 0)
        {
            printf(
                "  ✓ [%d] %s (pos: %d,%d size: %dx%d)\n", i, filepath,
                words_characters[i].x, words_characters[i].y,
                words_characters[i].w, words_characters[i].h
            );
        }
        else
        {
            printf("  ✗ [%d] Failed to save %s\n", i, filepath);
        }
    }
    */

    DrawingWand *draw = NewDrawingWand();

    // draw letters
    DrawLetterBoundries(wand, draw, grid_characters, grid_chars, "orange");
    DrawLetterBoundries(wand, draw, words_characters, words_chars, "yellow");

    // draw zones
    DrawZoneBoundries(draw, &ez.grid, "red");
    DrawZoneBoundries(draw, &ez.words, "green");

    printf(
        "Grid : x1:%i y1:%i x2:%i y2:%i\n", ez.grid.x_min, ez.grid.y_min,
        ez.grid.x_max, ez.grid.y_max
    );
    printf(
        "Wordlist: x1:%i y1:%i x2:%i y2:%i\n", ez.words.x_min, ez.words.y_min,
        ez.words.x_max, ez.words.y_max
    );

    free(grid_characters);
    free(words_characters);

    MagickDrawImage(wand, draw);

    MagickWriteImage(wand, output_path);
    DestroyDrawingWand(draw);

    DestroyMagickWand(wand);
    MagickWandTerminus();
}
