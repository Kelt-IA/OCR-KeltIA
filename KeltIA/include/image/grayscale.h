#pragma once
#include <MagickWand/MagickWand.h>

static double compute_average_gray(MagickWand *wand);
MagickBooleanType
binarize_image(const char *input_path, const char *output_path);
