#include "../include/image/grayscale.h"
#include "../include/image/removenoise.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 3 || argc > 4)
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
    int apply_removenoise = (argc == 4 && strcmp(argv[3], "-removenoise") == 0);

    MagickWandGenesis();

    // Step 1: Binarization (grayscale)
    printf("=== [1/2] Binarization / Grayscale ===\n");
    if (binarize_image(input_path, output_path) == MagickFalse)
    {
        fprintf(stderr, "Error: binarization failed.\n");
        MagickWandTerminus();
        return 1;
    }

    if (apply_removenoise)
    {
        // Reload the binarized image to continue processing
        MagickWand *wand = read_image(output_path);
        if (!wand)
        {
            fprintf(stderr, "Error: unable to reload binarized image.\n");
            MagickWandTerminus();
            return 1;
        }

        // Step 2: Noise Reduction
        printf("=== [2/2] Noise Reduction ===\n");
        MagickWand *clean = remove_noise(wand);
        if (!clean)
        {
            fprintf(stderr, "Error: noise reduction failed.\n");
            DestroyMagickWand(wand);
            MagickWandTerminus();
            return 1;
        }

        // Save the final result
        if (!write_image(clean, output_path))
        {
            fprintf(
                stderr, "Error: unable to write output image %s\n", output_path
            );
            DestroyMagickWand(wand);
            DestroyMagickWand(clean);
            MagickWandTerminus();
            return 1;
        }

        DestroyMagickWand(wand);
        DestroyMagickWand(clean);
    }
    else
    {
        printf("Skipping noise reduction (use -removenoise to enable it).\n");
    }

    printf("Image correction completed successfully: %s\n", output_path);

    MagickWandTerminus();
    return 0;
}
