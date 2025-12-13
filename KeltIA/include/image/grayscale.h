#pragma once
#include <MagickWand/MagickWand.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

static double compute_average_gray(MagickWand *wand);
MagickBooleanType
binarize_image(const char *input_path, const char *output_path);
