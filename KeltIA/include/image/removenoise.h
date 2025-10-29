#pragma once
#include <MagickWand/MagickWand.h>

static double compute_gray_stddev(MagickWand *wand);
static double auto_radius(double stddev);
MagickWand *remove_noise(const MagickWand *input_wand, double radius);
MagickWand *read_image(const char *path);
int write_image(MagickWand *wand, const char *path);
