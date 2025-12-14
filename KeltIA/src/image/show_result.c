#include "../../include/image/show_result.h"
#include <stdio.h>

void show_result(
    CharBBox **grid,
    char *char_grid,
    int height,
    int width,
    char **words,
    MagickWand *wand
)
{
    if (!words) return;

    for (int i = 0; words[i] != NULL; i++)
    {
        WordPos *pos = solver(
            char_grid, height, width, (const char *)words[i], strlen(words[i])
        );

        if (pos) { draw_word_box(grid, pos, wand, width, height); }
    }
}

void draw_word_box(
    CharBBox **grid,
    WordPos *pos,
    MagickWand *wand,
    int width,
    int height
)
{
    // Validate coordinates are within bounds
    if (pos->y1 < 0 || pos->y1 >= height || pos->y2 < 0 || pos->y2 >= height ||
        pos->x1 < 0 || pos->x1 >= width || pos->x2 < 0 || pos->x2 >= width)
    {
        fprintf(
            stderr,
            "Warning: WordPos coordinates out of bounds: "
            "(%d,%d) to (%d,%d), grid is %dx%d\n",
            pos->x1, pos->y1, pos->x2, pos->y2, width, height
        );
        free(pos);
        return;
    }

    CharBBox *start = grid[pos->y1] + pos->x1;
    CharBBox *end = grid[pos->y2] + pos->x2;

    printf(
        "Creating line frome %f %f to %f %f\n", start->x + (double)start->w / 2,
        start->y + (double)start->h / 2, end->x + (double)end->w / 2,
        end->y + (double)end->h / 2
    );

    free(pos);
    draw_line_between_letters(
        wand, start->x + (int)((double)start->w / 2),
        start->y + (int)((double)start->h / 2),
        end->x + (int)((double)end->w / 2), end->y + (int)((double)end->h / 2),
        5
    );
}

void draw_line_between_letters(
    MagickWand *image_wand,
    double x1,
    double y1,
    double x2,
    double y2,
    double thickness
)
{
    DrawingWand *draw_wand;
    PixelWand *stroke, *fill;

    draw_wand = NewDrawingWand();
    stroke = NewPixelWand();
    fill = NewPixelWand();

    PixelSetColor(stroke, "red");
    PixelSetColor(fill, "none");

    DrawSetStrokeColor(draw_wand, stroke);
    DrawSetFillColor(draw_wand, fill);
    DrawSetStrokeWidth(draw_wand, thickness);
    DrawSetStrokeAntialias(draw_wand, MagickTrue);
    DrawSetStrokeOpacity(draw_wand, 1.0);
    DrawSetStrokeLineCap(draw_wand, RoundCap);

    DrawPathStart(draw_wand);
    DrawPathMoveToAbsolute(draw_wand, x1, y1);
    DrawPathLineToAbsolute(draw_wand, x2, y2);
    DrawPathFinish(draw_wand);

    MagickDrawImage(image_wand, draw_wand);

    DestroyPixelWand(stroke);
    DestroyPixelWand(fill);
    DestroyDrawingWand(draw_wand);
}
