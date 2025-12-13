#include "../../include/detect_zones/detect_char.h"
#include "../../include/solver/solver.h"
#include <MagickWand/MagickWand.h>

void show_result(
    CharBBox **grid,
    char *char_grid,
    int height,
    int width,
    char **words,
    MagickWand *wand
);
void draw_word_box(CharBBox **grid, WordPos *pos, MagickWand *wand);
void draw_rotated_rectangle_diagonal(
    MagickWand *image_wand,
    double x1,
    double y1,
    double x2,
    double y2,
    double thickness
);
