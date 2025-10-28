#pragma once
#include <wand/MagickWand.h>

MagickWand *rotate_image(const MagickWand *input_wand, double angle);
