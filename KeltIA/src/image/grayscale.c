#include <MagickWand/MagickWand.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

// Compute the average gray level and return a normalized threshold (0.0–1.0)

static double compute_average_gray(MagickWand *wand)
{

    PixelIterator *iterator;
    PixelWand **pixels;
    size_t width, height;
    height = MagickGetImageHeight(wand);
    width = MagickGetImageWidth(wand);

    // 256-level histogram
    int hist[256] = {0};
    iterator = NewPixelIterator(wand);
    if (iterator == NULL) return 0.5;

    for (size_t y = 0; y < height; y++)
    {
        pixels = PixelGetNextIteratorRow(iterator, &width);
        if (pixels == NULL) break;
        for (size_t x = 0; x < width; x++)
        {
            double g = PixelGetRed(pixels[x]);
            int level = (int)(g * 255.0 + 0.5);
            if (level < 0) level = 0;
            if (level > 255) level = 255;
            hist[level]++;
        }
    }
    iterator = DestroyPixelIterator(iterator);

    size_t total = width * height;

    // Calculation of the optimal threshold (simplified Otsu method)
    double sum = 0;
    for (int i = 0; i < 256; i++) sum += i * hist[i];

    double sumB = 0;
    size_t wB = 0;
    double maxVar = 0;
    int threshold = 127;

    for (int i = 0; i < 256; i++)
    {
        wB += hist[i];
        if (wB == 0) continue;
        size_t wF = total - wB;
        if (wF == 0) break;

        sumB += (double)i * hist[i];
        double mB = sumB / wB;
        double mF = (sum - sumB) / wF;
        double varBetween = (double)wB * wF * (mB - mF) * (mB - mF);

        if (varBetween > maxVar)
        {
            maxVar = varBetween;
            threshold = i;
        }
    }

    // return a normalized value (0.0 - 1.0)
    return threshold / 255.0;
}

// Perform automatic binarization on an image.
//     - Convert image to grayscale
//     - Compute the optimal threshold (Otsu-like)
//     - Apply binary thresholding

MagickBooleanType
binarize_image(const char *input_path, const char *output_path)
{
    MagickWandGenesis();
    MagickWand *wand = NULL;
    MagickBooleanType status;
    double avg_gray, threshold_value, quantum_range;

    wand = NewMagickWand();
    if (MagickReadImage(wand, input_path) == MagickFalse)
    {
        fprintf(stderr, "Error: unable to read '%s'\n", input_path);
        if (wand) wand = DestroyMagickWand(wand);
        return MagickFalse;
    }

    MagickSetImageType(wand, GrayscaleType);

    // Compute optimal threshold
    avg_gray = compute_average_gray(wand);
    quantum_range = (double)QuantumRange;
    threshold_value = avg_gray * quantum_range;

    printf(
        "→ Automatic threshold: %.3f (%.0f out of %.0f)\n", avg_gray,
        threshold_value, quantum_range
    );

    // Apply binary threshold
    status = MagickThresholdImage(wand, threshold_value);
    if (status == MagickFalse)
    {
        fprintf(stderr, "Error: thresholding failed.\n");
        wand = DestroyMagickWand(wand);
        return MagickFalse;
    }

    // Save result
    if (MagickWriteImage(wand, output_path) == MagickFalse)
    {
        fprintf(stderr, "Error: unable to write '%s'\n", output_path);
        wand = DestroyMagickWand(wand);
        return MagickFalse;
    }

    printf("Binary image saved to: %s\n", output_path);
    wand = DestroyMagickWand(wand);
    MagickWandTerminus();
    return MagickTrue;
}
