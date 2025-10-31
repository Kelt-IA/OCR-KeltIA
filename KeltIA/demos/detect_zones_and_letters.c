#include "../include/detect_zones/detect_char.h"
#include "../include/detect_zones/detect_zones.h"
#include <err.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        errx(EXIT_FAILURE, "Usage: %s <input_image> <output_image>\n", argv[0]);
        errx(EXIT_FAILURE, "Detect the grid and the wordlist in a picture.\n");
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
