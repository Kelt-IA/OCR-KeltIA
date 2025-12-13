#pragma once
#include <MagickWand/MagickWand.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

MagickWand *remove_noise(MagickWand *wand);
MagickWand *read_image(const char *path);
int write_image(MagickWand *wand, const char *path);
// static double compute_gray_stddev(MagickWand *wand);
// static double auto_radius(double stddev);
