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

    // Step 1: Binarization (Otsu threshold)
    printf("=== [1/3] Otsu Binarization ===\n");
    MagickWand *wand = binarize_image_wand(input_path);
    if (!wand)
    {
        fprintf(stderr, "Error: binarization failed\n");
        MagickWandTerminus();
        return 1;
    }
    printf("  Binarization completed\n\n");

    // Step 2: Noise removal
    printf("=== [2/3] Noise Removal ===\n");
    MagickWand *clean = remove_noise(wand);
    if (!clean)
    {
        fprintf(stderr, "Error: noise removal failed\n");
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }
    printf("  Noise removal completed\n\n");

    // Free original wand, work with clean one
    DestroyMagickWand(wand);

    // Step 3: Auto-rotation
    printf("=== [3/3] Auto-rotation ===\n");
    MagickWand *rotated = auto_rotate_image(clean);
    if (!rotated)
    {
        fprintf(stderr, "Error: auto-rotation failed\n");
        DestroyMagickWand(clean);
        MagickWandTerminus();
        return 1;
    }
    printf("  Auto-rotation completed\n\n");

    // Free clean wand, work with rotated one
    DestroyMagickWand(clean);

    // Save final result
    printf("=== Saving Final Image ===\n");
    if (!write_image(rotated, output_path))
    {
        fprintf(stderr, "Error: cannot write %s\n", output_path);
        DestroyMagickWand(rotated);
        MagickWandTerminus();
        return 1;
    }

    printf("Success! Saved to: %s\n", output_path);

    DestroyMagickWand(rotated);
    MagickWandTerminus();

    return 0;
}
