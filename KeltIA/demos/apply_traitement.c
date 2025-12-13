#include "../include/image/apply_treatment.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <image_path>\n", argv[0]);
        fprintf(
            stderr, "Example: %s ressources/images_test/level_1_image_1.png\n",
            argv[0]
        );
        return EXIT_FAILURE;
    }

    const char *image_path = argv[1];

    // Initialiser ImageMagick une seule fois
    MagickWandGenesis();

    printf("=== Test du script apply_treatment.c ===\n");
    printf("Image: %s\n\n", image_path);

    // Test 1: Grayscale + binarisation + suppression bruit, pas de rotation
    printf("Test 1: Grayscale + binarisation + suppression bruit, pas de "
           "rotation\n");
    ProcessedImages *result1 = process_image(image_path, 0, 1, 1, 0, 66.0);
    if (result1)
    {
        printf("  Image NN: %s\n", result1->nn_image_path);
        printf("  Image UI: %s\n", result1->ui_image_path);
        free(result1->nn_image_path);
        free(result1->ui_image_path);
        free(result1);
    }
    printf("✓ Test 1 terminé\n\n");

    while (1) {}
    printf("=== Tous les tests terminés ===\n");
    printf("Les images traitées sont sauvegardées dans /tmp/\n");

    // Terminer ImageMagick
    MagickWandTerminus();

    return EXIT_SUCCESS;
}
