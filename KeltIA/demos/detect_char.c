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

    // Détecter caractères dans la grille
    int grid_chars = 0;
    CharBBox *grid_characters = detect_characters(
        wand, ez.grid.x_min, ez.grid.y_min, ez.grid.x_max - ez.grid.x_min,
        ez.grid.y_max - ez.grid.y_min, &grid_chars
    );

    // Détecter mots dans la liste
    int words_chars = 0;
    CharBBox *words_characters = detect_characters(
        wand, ez.words.x_min, ez.words.y_min, ez.words.x_max - ez.words.x_min,
        ez.words.y_max - ez.words.y_min, &words_chars
    );

    DrawingWand *draw = NewDrawingWand();
    PixelWand *stroke_color = NewPixelWand();
    PixelWand *fill_color = NewPixelWand();

    PixelSetColor(fill_color, "none");

    PixelSetColor(stroke_color, "blue");
    DrawSetStrokeColor(draw, stroke_color);
    DrawSetStrokeWidth(draw, 2);

    PixelSetColor(fill_color, "none");
    DrawSetFillColor(draw, fill_color);

    for (int i = 0; i < grid_chars; i++)
    {
        DrawRectangle(
            draw, grid_characters[i].x, grid_characters[i].y,
            grid_characters[i].x + grid_characters[i].w,
            grid_characters[i].y + grid_characters[i].h
        );
    }

    free(grid_characters);

    PixelSetColor(stroke_color, "green");
    DrawSetStrokeColor(draw, stroke_color);

    for (int i = 0; i < words_chars; i++)
    {
        DrawRectangle(
            draw, words_characters[i].x, words_characters[i].y,
            words_characters[i].x + words_characters[i].w,
            words_characters[i].y + words_characters[i].h
        );
    }

    free(words_characters);

    MagickDrawImage(wand, draw);

    MagickWriteImage(wand, output_path);
    DestroyPixelWand(stroke_color);
    DestroyPixelWand(fill_color);
    DestroyDrawingWand(draw);

    DestroyMagickWand(wand);
    MagickWandTerminus();
}
