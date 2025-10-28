/* src/image/grayscale.c
   Implementation du seuillage (grayscale -> binaire)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/image/grayscale.h"

#include <MagickWand/MagickWand.h>

/* Helper to normalize threshold to 0..1 */
static double normalize_threshold(double thr)
{
    if (thr <= 0.0) return 0.0;
    if (thr > 1.0)
    {
        /* Suppose input in 0..255 range */
        return thr / 255.0;
    }
    return thr;
}

MagickBooleanType grayscale_threshold_wand(MagickWand *wand, double threshold)
{
    if (wand == NULL) return MagickFalse;

    MagickBooleanType status = MagickTrue;
    double thr = normalize_threshold(threshold);

    /* Convert image to grayscale colorspace first */
    status = MagickTransformImageColorspace(wand, GRAYColorspace);
    if (status == MagickFalse) { return MagickFalse; }

    /* Get ImageMagick quantum range (max value for pixel sample) */
    size_t quantum_range = 0;
    /* MagickGetQuantumRange returns a const char *, but also writes the numeric
     * range into the provided pointer */
    MagickGetQuantumRange(&quantum_range);

    /* compute threshold in quantum units */
    double threshold_value = thr * (double)quantum_range;

    /* Apply threshold: pixels < threshold become white, others black (per
     * MagickThresholdImage doc) */
    status = MagickThresholdImage(wand, threshold_value);

    return status;
}

int grayscale_threshold_file(
    const char *in_path,
    const char *out_path,
    double threshold
)
{
    MagickWand *wand = NULL;
    MagickBooleanType status;
    int ret = 0;

    if (in_path == NULL || out_path == NULL) return 1;

    MagickWandGenesis();
    wand = NewMagickWand();
    if (wand == NULL)
    {
        MagickWandTerminus();
        return 2;
    }

    status = MagickReadImage(wand, in_path);
    if (status == MagickFalse)
    {
        fprintf(stderr, "Erreur : impossible de lire l'image '%s'\n", in_path);
        ret = 3;
        goto cleanup;
    }

    status = grayscale_threshold_wand(wand, threshold);
    if (status == MagickFalse)
    {
        fprintf(stderr, "Erreur : échec du seuillage.\n");
        ret = 4;
        goto cleanup;
    }

    status = MagickWriteImage(wand, out_path);
    if (status == MagickFalse)
    {
        fprintf(
            stderr, "Erreur : impossible d'écrire l'image '%s'\n", out_path
        );
        ret = 5;
        goto cleanup;
    }

cleanup:
    if (wand) wand = DestroyMagickWand(wand);
    MagickWandTerminus();
    return ret;
}
