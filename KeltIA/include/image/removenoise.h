#pragma once
#include <MagickWand/MagickWand.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

MagickWand *remove_noise(MagickWand *wand);
MagickWand *remove_noise_adaptive(MagickWand *wand, double strength);
MagickWand *clean_binary_image(MagickWand *wand);
MagickWand *read_image(const char *path);
int write_image(MagickWand *wand, const char *path);
double compute_gray_stddev(MagickWand *wand);
double auto_radius(double stddev);

double detect_fine_lines(MagickWand *wand);
double auto_strength(MagickWand *wand);
