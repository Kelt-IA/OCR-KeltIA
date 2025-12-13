#pragma once
#include <MagickWand/MagickWand.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>


double compute_average_gray(MagickWand *wand);
MagickBooleanType
binarize_image(const char *input_path, const char *output_path);

// Load and binarize image, return MagickWand (in-memory, no file save)
MagickWand *binarize_image_wand(const char *input_path);

// Keep the original function for backwards compatibility
MagickBooleanType
binarize_image(const char *input_path, const char *output_path);
