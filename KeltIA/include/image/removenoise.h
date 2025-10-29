#pragma once
#include <MagickWand/MagickWand.h>

MagickWand *remove_noise(const MagickWand *input_wand);
MagickWand *read_image(const char *path);
int write_image(MagickWand *wand, const char *path);
