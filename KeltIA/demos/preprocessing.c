#include "../include/image/autorotation.h"
#include "../include/image/grayscale.h"
#include "../include/image/removenoise.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input_image> <output_image>\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];

    MagickWandGenesis();

    printf("=== Image Preprocessing Pipeline ===\n");
    printf("Input: %s\n", input_path);
    printf("Output: %s\n\n", output_path);

    // Load
    MagickWand *wand = read_image(input_path);
    if (!wand)
    {
        MagickWandTerminus();
        return 1;
    }

    // Step 1: Grayscale + Otsu Binarization
    printf("=== [1/3] Otsu Binarization ===\n");
    if (apply_otsu_binarization(wand) == MagickFalse)
    {
        fprintf(stderr, "Error: binarization failed\n");
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }
    printf("  Binarization completed\n\n");

    // Step 2: Remove small noise
    printf("=== [2/3] Remove Small Noise ===\n");
    remove_small_noise(wand);
    printf("  Noise cleanup completed\n\n");

    // Step 3: Auto-rotation
    printf("=== [3/3] Auto-rotation ===\n");
    MagickWand *rotated = auto_rotate_image(wand);
    if (!rotated)
    {
        fprintf(stderr, "Error: auto-rotation failed\n");
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }
    printf("  Auto-rotation completed\n\n");

    // Save
    printf("=== Saving Final Image ===\n");
    if (!write_image(rotated, output_path))
    {
        fprintf(stderr, "Error: cannot write %s\n", output_path);
        DestroyMagickWand(wand);
        DestroyMagickWand(rotated);
        MagickWandTerminus();
        return 1;
    }

    printf("Success! Saved to: %s\n", output_path);

    DestroyMagickWand(wand);
    DestroyMagickWand(rotated);
    MagickWandTerminus();

    return 0;
}
