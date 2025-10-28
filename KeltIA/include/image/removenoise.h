#pragma once
#include <MagickWand/MagickWand.h>

MagickWand *remove_noise(const MagickWand *input_wand, double radius);
