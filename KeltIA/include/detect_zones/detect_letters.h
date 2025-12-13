#include "../../include/detect_zones/detect_zones.h"

#include "detect_char.h"
#include <MagickWand/MagickWand.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>

CharBBox *detect_letters(
    MagickWand *wand,
    int x_offset,
    int y_offset,
    int width,
    int height,
    int *char_count
);
