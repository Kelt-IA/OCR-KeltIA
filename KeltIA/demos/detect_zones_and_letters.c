#include "../include/detect_zones/detect_char.h"
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

    // MagickDrawImage(wand, draw);

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

    DrawingWand *draw = NewDrawingWand();

    // draw letters
    DrawLetterBoundries(wand, draw, grid_characters, grid_chars, "blue");
    DrawLetterBoundries(wand, draw, words_characters, words_chars, "blue");

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

/*
// demos/extract_and_save_letters.c
#include "../include/detect_zones/detect_char.h"
#include "../include/detect_zones/detect_zones.h"
#include "../include/image/grayscale.h"
#include "../include/image/removenoise.h"
#include <MagickWand/MagickWand.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

// Save character as image file
static void save_char_image(
    MagickWand *wand,
    CharBBox *bbox,
    int zone_x,
    int zone_y,
    const char *output_path
)
{
    MagickWand *char_wand = CloneMagickWand(wand);

    int abs_x = zone_x + bbox->x;
    int abs_y = zone_y + bbox->y;

    MagickCropImage(char_wand, bbox->w, bbox->h, abs_x, abs_y);
    MagickResetImagePage(char_wand, "");

    MagickWriteImage(char_wand, output_path);
    DestroyMagickWand(char_wand);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input_image> <output_folder>\n", argv[0]);
        fprintf(stderr, "Example: %s crossword.png extracted\n", argv[0]);
        fprintf(stderr, "\nLetters will be saved to:\n");
        fprintf(stderr, "  <output_folder>/grid/\n");
        fprintf(stderr, "  <output_folder>/list/\n");
        return EXIT_FAILURE;
    }

    const char *input_path = argv[1];
    const char *output_base = argv[2];

    printf("╔════════════════════════════════════════╗\n");
    printf("║    Extract and Save Letter Images    ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    // Create output directories
    char grid_dir[512], list_dir[512];
    snprintf(grid_dir, sizeof(grid_dir), "%s/grid", output_base);
    snprintf(list_dir, sizeof(list_dir), "%s/list", output_base);

    mkdir(output_base, 0755);
    mkdir(grid_dir, 0755);
    mkdir(list_dir, 0755);

    printf("Output directories:\n");
    printf("  Grid: %s/\n", grid_dir);
    printf("  List: %s/\n\n", list_dir);

    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    if (!MagickReadImage(wand, input_path))
    {
        DestroyMagickWand(wand);
        MagickWandTerminus();
        errx(EXIT_FAILURE, "Erreur: Could not read %s\n", input_path);
    }

    // Step 2: Detect zones and characters
    printf("[2/3] Detecting zones and characters...\n");
    ExtractedZones ez = detect_zones(wand);

    int grid_count = 0;
    CharBBox *grid_chars = detect_characters(
        wand, ez.grid.x_min, ez.grid.y_min, ez.grid.x_max - ez.grid.x_min,
        ez.grid.y_max - ez.grid.y_min, &grid_count
    );

    int list_count = 0;
    CharBBox *list_chars = detect_characters(
        wand, ez.words.x_min, ez.words.y_min, ez.words.x_max - ez.words.x_min,
        ez.words.y_max - ez.words.y_min, &list_count
    );

    printf("  Grid: %d characters\n", grid_count);
    printf("  List: %d characters\n", list_count);
    printf("✓ Done\n\n");

    // Step 3: Save character images
    printf("[3/3] Saving character images...\n");

    // Save grid characters
    for (int i = 0; i < grid_count; i++)
    {
        char filename[1024];  // ← Cambia de 512 a 1024
        snprintf(
            filename, sizeof(filename), "%s/grid_char_%04d.png", grid_dir, i
        );
        save_char_image(
            wand, &grid_chars[i], ez.grid.x_min, ez.grid.y_min, filename
        );
    }
    printf("  ✓ Saved %d grid characters to %s/\n", grid_count, grid_dir);

    // Save list characters
    for (int i = 0; i < list_count; i++)
    {
        char filename[1024];  // ← Cambia de 512 a 1024
        snprintf(
            filename, sizeof(filename), "%s/list_char_%04d.png", list_dir, i
        );
        save_char_image(
            wand, &list_chars[i], ez.words.x_min, ez.words.y_min, filename
        );
    }

    printf("  ✓ Saved %d list characters to %s/\n", list_count, list_dir);

    printf("✓ Done\n\n");

    // Cleanup
    free(grid_chars);
    free(list_chars);
    DestroyMagickWand(wand);
    MagickWandTerminus();

    printf("╔════════════════════════════════════════╗\n");
    printf("║         Extraction Complete!          ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    printf("Check the output folders:\n");
    printf("  %s/\n", grid_dir);
    printf("  %s/\n\n", list_dir);

    return EXIT_SUCCESS;
}
*/
