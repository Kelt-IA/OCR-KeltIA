#pragma once
#include <MagickWand/MagickWand.h>
#include <stdio.h>

MagickWand *rotate_image(const MagickWand *input_wand, double angle);
