#pragma once

#include <MagickWand/MagickWand.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    int *horizontal, *vertical;
} Projections;

typedef struct
{
    int x_min, y_min, x_max, y_max;
} BoundingBox;

typedef struct
{
    BoundingBox grid;
    BoundingBox words;
} ExtractedZones;

typedef struct
{
    int start, end;
    int density;
} Zone;

char is_pixel_black(
    MagickWand *wand,
    size_t x,
    size_t y,
    PixelWand *pixel_wand
);
Projections *projection(MagickWand *wand);
Zone *find_two_main_zones(int *proj, int size, int *count);
BoundingBox extract_bbox_horizontal(MagickWand *wand, int y_min, int y_max);
BoundingBox extract_bbox_vertical(MagickWand *wand, int x_min, int x_max);
ExtractedZones
extract_zones_vertical(MagickWand *wand, Zone *zones, int zone_count);
ExtractedZones
extract_zones_horizontal(MagickWand *wand, Zone *zones, int zone_count);
char detect_layout(
    MagickWand *wand,
    Projections **projs,
    Zone **zone,
    int *count
);
ExtractedZones detect_zones(MagickWand *wand);
