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
            stderr, "Usage: %s <input_image> <output_image> [-removenoise]\n",
            argv[0]
        );
        fprintf(
            stderr,
            "Applies grayscale binarization and optionally noise reduction.\n"
        );
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];
    int apply_removenoise = 0;

    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "-removenoise") == 0) { apply_removenoise = 1; }
        else
        {
            fprintf(stderr, "Warning: unknown option '%s' ignored.\n", argv[i]);
        }
    }

    MagickWandGenesis();

    MagickWand *wand = read_image(input_path);
    if (!wand)
    {
        fprintf(stderr, "Error: unable to read input image.\n");
        MagickWandTerminus();
        return 1;
    }

    // Step 1: Binarization (grayscale)
    printf("=== [1/2] Binarization / Grayscale ===\n");
    MagickSetImageType(wand, GrayscaleType);

    double avg_gray = compute_average_gray(wand);
    double quantum_range = (double)QuantumRange;
    double threshold_value = avg_gray * quantum_range;

    printf(
        "→ Automatic threshold: %.3f (%.0f out of %.0f)\n", avg_gray,
        threshold_value, quantum_range
    );

    if (MagickThresholdImage(wand, threshold_value) == MagickFalse)
    {
        fprintf(stderr, "Error: thresholding failed.\n");
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    // Step 2: Optional Noise Reduction (AFTER binarization)
    if (apply_removenoise)
    {
        printf("=== [2/2] Noise Reduction ===\n");
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
    }
    else
    {
        printf("Skipping noise reduction (use -removenoise to enable it).\n");
    }

    // Save the final result
    if (!write_image(wand, output_path))
    {
        fprintf(
            stderr, "Error: unable to write output image %s\n", output_path
        );
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    DestroyMagickWand(wand);

    printf("Image correction completed successfully: %s\n", output_path);

    MagickWandTerminus();
    return 0;
}
