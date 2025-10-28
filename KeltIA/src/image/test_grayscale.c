#include "../include/image/grayscale.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        fprintf(
            stderr, "Usage: %s <image_entree> <image_sortie> <seuil>\n", argv[0]
        );
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_path = argv[2];
    double threshold = atof(argv[3]);

    int result = grayscale_threshold_file(input_path, output_path, threshold);
    if (result != 0)
    {
        fprintf(
            stderr, "Erreur lors du traitement de l'image (code %d)\n", result
        );
        return result;
    }

    printf("Image traitée avec succès : %s -> %s\n", input_path, output_path);
    return 0;
}
