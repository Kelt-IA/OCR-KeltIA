#pragma once
#include <MagickWand/MagickWand.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

MagickWand *auto_rotate_image(MagickWand *input_wand);
MagickWand *
auto_rotate_image_bruteforce(MagickWand *input_wand, double angle_step);
