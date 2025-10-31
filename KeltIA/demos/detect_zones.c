#include "../include/detect_zones/detect_zones.h"
#include "MagickWand/pixel-wand.h"
#include <err.h>

int main(int argc, char *argv[])
{
    if (argc < 3 || argc > 4)
    {
        fprintf(stderr, "Usage: %s <input_image> <output_image>\n", argv[0]);
        fprintf(
            stderr,
            "Applies grayscale binarization and optionally noise reduction.\n"
        );
        return EXIT_FAILURE;
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

    DrawingWand *draw = NewDrawingWand();
    PixelWand *stroke_color = NewPixelWand();
    PixelWand *fill_color = NewPixelWand();

    PixelSetColor(fill_color, "white");

    PixelSetColor(stroke_color, "red");
    DrawSetStrokeColor(draw, stroke_color);
    DrawSetStrokeWidth(draw, 3);

    PixelSetColor(fill_color, "none");
    DrawSetFillColor(draw, fill_color);

    DrawRectangle(
        draw, ez.grid.x_min, ez.grid.y_min, ez.grid.x_max, ez.grid.y_max
    );

    PixelSetColor(stroke_color, "green");
    DrawSetStrokeColor(draw, stroke_color);
    DrawRectangle(
        draw, ez.words.x_min, ez.words.y_min, ez.words.x_max, ez.words.y_max
    );

    MagickDrawImage(wand, draw);

    printf(
        "Grid : x1:%i y1:%i x2:%i y2:%i\n", ez.grid.x_min, ez.grid.y_min,
        ez.grid.x_max, ez.grid.y_max
    );
    printf(
        "Wordlist: x1:%i y1:%i x2:%i y2:%i\n", ez.words.x_min, ez.words.y_min,
        ez.words.x_max, ez.words.y_max
    );

    MagickWriteImage(wand, output_path);
    DestroyPixelWand(stroke_color);
    DestroyPixelWand(fill_color);
    DestroyDrawingWand(draw);

    DestroyMagickWand(wand);
    MagickWandTerminus();
}
