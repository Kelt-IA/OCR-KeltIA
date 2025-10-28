#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

MagickWand *rotate_image(const MagickWand *input_wand, double angle)
{
    MagickWand *rotated_wand = NULL;
    MagickBooleanType status;
    PixelWand *background = NULL;

    // Cloner l’image d’entrée
    rotated_wand = CloneMagickWand(input_wand);
    if (rotated_wand == NULL)
    {
        fprintf(stderr, "Error: unable to clone the input MagickWand.\n");
        return NULL;
    }

    // Définir la couleur de fond (blanc)
    background = NewPixelWand();
    PixelSetColor(background, "white");

    // Effectuer la rotation (3 arguments seulement)
    status = MagickRotateImage(rotated_wand, background, angle);
    if (status == MagickFalse)
    {
        fprintf(stderr, "Error: failed to rotate the image.\n");
        if (rotated_wand) rotated_wand = DestroyMagickWand(rotated_wand);
        if (background) background = DestroyPixelWand(background);
        return NULL;
    }

    // Nettoyer
    if (background) background = DestroyPixelWand(background);

    return rotated_wand;
}

// Fonction principale
int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <input_image> <output_image> <angle>\n",
                argv[0]);
        return 1;
    }

    const char *input_file = argv[1];
    const char *output_file = argv[2];
    double angle = atof(argv[3]);

    MagickWand *wand = NULL;
    MagickWand *rotated = NULL;

    // Initialisation de MagickWand
    MagickWandGenesis();

    wand = NewMagickWand();
    if (MagickReadImage(wand, input_file) == MagickFalse)
    {
        fprintf(stderr, "Error: unable to read input image %s\n", input_file);
        if (wand) wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    rotated = rotate_image(wand, angle);
    if (rotated == NULL)
    {
        fprintf(stderr, "Error: rotation failed.\n");
        if (wand) wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    if (MagickWriteImage(rotated, output_file) == MagickFalse)
    {
        fprintf(stderr, "Error: unable to write output image %s\n",
                output_file);
        if (rotated) rotated = DestroyMagickWand(rotated);
        if (wand) wand = DestroyMagickWand(wand);
        MagickWandTerminus();
        return 1;
    }

    printf("Image rotated by %.2f degrees and saved to %s\n", angle,
           output_file);

    // Nettoyage
    if (rotated) rotated = DestroyMagickWand(rotated);
    if (wand) wand = DestroyMagickWand(wand);
    MagickWandTerminus();

    return 0;
}