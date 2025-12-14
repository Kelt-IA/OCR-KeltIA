#include "../../include/image/show_result.h"

void show_result(
    CharBBox **grid,
    char *char_grid,
    int height,
    int width,
    char **words,
    MagickWand *wand
)
{
    // Check if words array exists
    if (!words) return;

    // Iterate through null-terminated array
    for (int i = 0; words[i] != NULL; i++)  // ← FIX: Check if words[i] is NULL
    {
        WordPos *pos = solver(
            char_grid, height, width,
            (const char *)words[i],  // ← Use words[i] directly
            strlen(words[i])
        );

        if (pos) { draw_word_box(grid, pos, wand); }
    }

    // Don't free words here! The caller should do it
    // Because you're not freeing the individual words anymore
}

void draw_word_box(CharBBox **grid, WordPos *pos, MagickWand *wand)
{
    CharBBox *start = grid[pos->y1] + pos->x1;
    CharBBox *end = grid[pos->y2] + pos->x2;
    free(pos);
    /* Calculate center coordinates of start and end characters */
    double start_center_x = start->x + start->w / 2.0;
    double start_center_y = start->y + start->h / 2.0;
    double end_center_x = end->x + end->w / 2.0;
    double end_center_y = end->y + end->h / 2.0;

    /* Use average height as rectangle thickness */
    double avg_height = (start->h + end->h) / 2.0;

    draw_rotated_rectangle_diagonal(
        wand, start_center_x, start_center_y, end_center_x, end_center_y,
        avg_height /* Thickness based on character height */
    );
}

void draw_rotated_rectangle_diagonal(
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
    PointInfo points[4];
    double dx, dy, length, half_w;

    /* Create drawing wand and colors */
    draw_wand = NewDrawingWand();
    stroke = NewPixelWand();
    fill = NewPixelWand();

    /* Example colors: red outline, transparent fill */
    PixelSetColor(stroke, "red");
    PixelSetColor(fill, "none");
    DrawSetStrokeColor(draw_wand, stroke);
    DrawSetStrokeWidth(draw_wand, 1.0); /* outline width */
    DrawSetFillColor(draw_wand, fill);

    dx = x2 - x1;
    dy = y2 - y1;
    length = sqrt(dx * dx + dy * dy);

    if (length == 0.0)
    {
        /* Avoid division by zero */
        DestroyPixelWand(stroke);
        DestroyPixelWand(fill);
        DestroyDrawingWand(draw_wand);
        return;
    }

    half_w = thickness / 2.0;

    /* perpendicular unit vector */
    double px = -dy / length;
    double py = dx / length;

    /* Four corners of the rectangle */
    points[0].x = x1 + px * half_w;
    points[0].y = y1 + py * half_w;
    points[1].x = x2 + px * half_w;
    points[1].y = y2 + py * half_w;
    points[2].x = x2 - px * half_w;
    points[2].y = y2 - py * half_w;
    points[3].x = x1 - px * half_w;
    points[3].y = y1 - py * half_w;

    /* Draw polygon (4 points) on the drawing wand */
    DrawPolygon(draw_wand, 4, points);

    /* Apply drawing operations to the image */
    MagickDrawImage(image_wand, draw_wand);

    DestroyPixelWand(stroke);
    DestroyPixelWand(fill);
    DestroyDrawingWand(draw_wand);
}
