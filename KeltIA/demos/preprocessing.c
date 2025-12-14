#include "../include/image/autorotation.h"
#include "../include/image/grayscale.h"
#include "../include/image/removenoise.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(
            stderr, "Usage: %s [-removenoise]\n",
            argv[0]
        );
        fprintf(
            stderr,
            "Applies grayscale binarization with automatic threshold, "
            "optional noise reduction, and auto-rotation.\n"
        );
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];
    int apply_removenoise = 0;

    // Parse options
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-removenoise") == 0) 
        { 
            apply_removenoise = 1; 
        }
        else
        {
            fprintf(stderr, "Warning: unknown option '%s' ignored.\n", argv[i]);
        }
    }

    MagickWandGenesis();
    printf("=== Image Preprocessing Pipeline ===\n");
    printf("Input: %s\n", input_path);
    printf("Output: %s\n\n", output_path);

    // Read input image
    MagickWand *wand = read_image(input_path);
    if (!wand)
    {
        fprintf(stderr, "Error: unable to read input image.\n");
        MagickWandTerminus();
        return 1;
    }

    // Step 1: Binarization with automatic threshold (from image_correction)
    printf("=== [1/4] Binarization / Grayscale ===\n");
    MagickSetImageType(wand, GrayscaleType);
    double avg_gray = compute_average_gray(wand);
    double quantum_range = (double)QuantumRange;
    double threshold_value = avg_gray * quantum_range;
    printf(
        "→ Automatic threshold: %.3f (%.0f out of %.0f)\n", 
        avg_gray,
        threshold_value, 
        quantum_range
    );

    if (MagickThresholdImage(wand, threshold_value) == MagickFalse)
    {
        fprintf(stderr, "Error: thresholding failed.\n");
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }
    printf(" Binarization completed\n\n");

    // Step 2: Optional Noise Reduction (from image_correction)
    if (apply_removenoise)
    {
        printf("=== [2/4] Noise Reduction ===\n");
        MagickWand *clean = clean_binary_image(wand);
        if (!clean)
        {
            fprintf(stderr, "Error: noise reduction failed.\n");
            DestroyMagickWand(wand);
            MagickWandTerminus();
            return 1;
        }
        DestroyMagickWand(wand);
        wand = clean;
        printf(" Noise removal completed\n\n");
    }
    else
    {
        printf("=== [2/4] Noise Reduction ===\n");
        printf("Skipping noise reduction (use -removenoise to enable it).\n\n");
    }

    // Step 3: Standard noise removal (from original preprocessing)
    printf("=== [3/4] Standard Noise Removal ===\n");
    MagickWand *clean = remove_noise(wand);
    if (!clean)
    {
        fprintf(stderr, "Error: noise removal failed\n");
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }
    printf(" Noise removal completed\n\n");
    DestroyMagickWand(wand);

    // Step 4: Auto-rotation (from original preprocessing)
    printf("=== [4/4] Auto-rotation ===\n");
    MagickWand *rotated = auto_rotate_image(clean);
    if (!rotated)
    {
        fprintf(stderr, "Error: auto-rotation failed\n");
        DestroyMagickWand(clean);
        MagickWandTerminus();
        return 1;
    }
    printf(" Auto-rotation completed\n\n");
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
