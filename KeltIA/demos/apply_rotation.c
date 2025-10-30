#include "../include/image/removenoise.h"
#include "../include/image/rotation.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(
            stderr, "Usage: %s <input_image> <output_image> <angle>\n", argv[0]
        );
        fprintf(
            stderr, "Rotates an image by the specified angle (in degrees).\n"
        );
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];
    double angle = atof(argv[3]);

    MagickWandGenesis();

    MagickWand *wand = read_image(input_path);
    if (!wand)
    {
        fprintf(stderr, "Error: unable to read input image %s\n", input_path);
        MagickWandTerminus();
        return 1;
    }

    printf("=== Rotating image by %.2f degrees ===\n", angle);
    MagickWand *rotated = rotate_image(wand, angle);
    if (!rotated)
    {
        fprintf(stderr, "Error: rotation failed.\n");
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    if (!write_image(rotated, output_path))
    {
        fprintf(
            stderr, "Error: unable to write output image %s\n", output_path
        );
        DestroyMagickWand(wand);
        DestroyMagickWand(rotated);
        MagickWandTerminus();
        return 1;
    }

    printf("Rotation completed successfully: %s\n", output_path);

    DestroyMagickWand(wand);
    DestroyMagickWand(rotated);
    MagickWandTerminus();
    return 0;
}
