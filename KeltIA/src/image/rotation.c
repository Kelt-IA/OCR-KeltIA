#include "../include/image/rotation.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

MagickWand *rotate_image(const MagickWand *input_wand, double angle)
{
    MagickWand *rotated_wand = NULL;
    MagickBooleanType status;
    PixelWand *background = NULL;

    // Clone the input image
    rotated_wand = CloneMagickWand(input_wand);
    if (rotated_wand == NULL)
    {
        fprintf(stderr, "Error: unable to clone the input MagickWand.\n");
        return NULL;
    }

    // Define the background color (white)
    background = NewPixelWand();
    PixelSetColor(background, "white");

    // perform the rotation (only 3 arguments)
    status = MagickRotateImage(rotated_wand, background, angle);
    if (status == MagickFalse)
    {
        fprintf(stderr, "Error: failed to rotate the image.\n");
        if (rotated_wand) rotated_wand = DestroyMagickWand(rotated_wand);
        if (background) background = DestroyPixelWand(background);
        return NULL;
    }

    // cleanup
    if (background) background = DestroyPixelWand(background);

    return rotated_wand;
}
