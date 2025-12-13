#include "../include/image/autorotation.h"
#include "../include/image/removenoise.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Test program for automatic image rotation
 * Usage: ./autorotation_test <input_image> <output_image>
 */
int main(int argc, char *argv[])
{
    MagickWand *input_wand = NULL;
    MagickWand *output_wand = NULL;
    MagickBooleanType status;

    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <input_image> <output_image>\n", argv[0]);
        fprintf(
            stderr, "Example: %s rotated_grid.png corrected_grid.png\n", argv[0]
        );
        return 1;
    }

    MagickWandGenesis();

    input_wand = NewMagickWand();
    if (input_wand == NULL)
    {
        fprintf(stderr, "Error: cannot create MagickWand.\n");
        MagickWandTerminus();
        return 1;
    }

    printf("Loading image: %s\n", argv[1]);
    status = MagickReadImage(input_wand, argv[1]);
    if (status == MagickFalse)
    {
        fprintf(stderr, "Error: cannot read image '%s'.\n", argv[1]);
        char *error_msg = MagickGetException(input_wand, NULL);
        fprintf(stderr, "MagickWand error: %s\n", error_msg);
        error_msg = MagickRelinquishMemory(error_msg);
        input_wand = DestroyMagickWand(input_wand);
        MagickWandTerminus();
        return 1;
    }

    printf("Image loaded successfully.\n");
    printf(
        "Image dimensions: %lux%lu\n", MagickGetImageWidth(input_wand),
        MagickGetImageHeight(input_wand)
    );

    printf("\nApplying auto-rotation...\n");
    output_wand = auto_rotate_image(input_wand);

    if (output_wand == NULL)
    {
        fprintf(stderr, "Error: auto-rotation failed.\n");
        input_wand = DestroyMagickWand(input_wand);
        MagickWandTerminus();
        return 1;
    }

    printf("Auto-rotation completed successfully.\n");

    printf("Saving corrected image to: %s\n", argv[2]);
    status = MagickWriteImage(output_wand, argv[2]);

    if (status == MagickFalse)
    {
        fprintf(stderr, "Error: cannot write image '%s'.\n", argv[2]);
        char *error_msg = MagickGetException(output_wand, NULL);
        fprintf(stderr, "MagickWand error: %s\n", error_msg);
        error_msg = MagickRelinquishMemory(error_msg);
    }
    else
    {
        printf("Image saved successfully!\n");
    }

    if (input_wand) input_wand = DestroyMagickWand(input_wand);
    if (output_wand) output_wand = DestroyMagickWand(output_wand);

    MagickWandTerminus();

    return 0;
}
