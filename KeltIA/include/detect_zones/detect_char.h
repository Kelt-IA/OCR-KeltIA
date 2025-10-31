#include "../../include/detect_zones/detect_zones.h"

#include <MagickWand/MagickWand.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int x, y, w, h;
} CharBBox;

typedef struct
{
    int x, y;
} Point;

void flood_fill(
    MagickWand *wand,
    int start_x,
    int start_y,
    char *visited,
    int width,
    int height,
    int x_offset,
    int y_offset,
    int *x_min,
    int *x_max,
    int *y_min,
    int *y_max
);

CharBBox *detect_characters(
    MagickWand *wand,
    int x_offset,
    int y_offset,
    int width,
    int height,
    int *char_count
);

MagickWand *extract_zone(MagickWand *wand, int x, int y, int width, int height);
