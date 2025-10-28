#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

MagickWand *remove_noise(const MagickWand *input_wand, double radius)
{
    if (input_wand == NULL)
    {
        fprintf(stderr, "Erreur : input_wand est NULL.\n");
        return NULL;
    }

    MagickWand *clean_wand = CloneMagickWand(input_wand);
    if (clean_wand == NULL)
    {
        fprintf(stderr, "Erreur : impossible de cloner l'image source.\n");
        return NULL;
    }

    // Appliquer un filtre de type moyenne locale pour réduire le bruit
    MagickBooleanType status =
        MagickStatisticImage(clean_wand,
                             MedianStatistic,  // filtre médian
                             radius, radius);

    if (status == MagickFalse)
    {
        fprintf(stderr, "Erreur : échec du débruitage (radius=%.2f).\n",
                radius);
        clean_wand = DestroyMagickWand(clean_wand);
        return NULL;
    }

    return clean_wand;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <input> <output> <radius>\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];
    double radius = atof(argv[3]);

    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    if (MagickReadImage(wand, input_path) == MagickFalse)
    {
        fprintf(stderr, "Erreur : impossible de lire %s\n", input_path);
        MagickWandTerminus();
        return 1;
    }

    MagickWand *denoised = remove_noise(wand, radius);
    if (denoised)
    {
        MagickWriteImage(denoised, output_path);
        printf("Image débruitée enregistrée sous %s\n", output_path);
        DestroyMagickWand(denoised);
    }

    DestroyMagickWand(wand);
    MagickWandTerminus();
    return 0;
}