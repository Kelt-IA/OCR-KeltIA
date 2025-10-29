#include <MagickWand/MagickWand.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Estimation of the variance of grey levels
static double compute_gray_stddev(MagickWand *wand)
{
    PixelIterator *iterator;
    PixelWand **pixels;
    size_t width, height;
    height = MagickGetImageHeight(wand);
    width = MagickGetImageWidth(wand);

    iterator = NewPixelIterator(wand);
    if (iterator == NULL) return 0.0;

    double sum = 0.0;
    double sumsq = 0.0;
    size_t count = 0;

    for (size_t y = 0; y < height; y++)
    {
        pixels = PixelGetNextIteratorRow(iterator, &width);
        if (pixels == NULL) break;

        for (size_t x = 0; x < width; x++)
        {
            double g = PixelGetRed(pixels[x]);  // image in grayscale
            double val = g * 255.0;
            sum += val;
            sumsq += val * val;
            count++;
        }
    }

    iterator = DestroyPixelIterator(iterator);

    if (count == 0) return 0.0;

    double mean = sum / count;
    double variance = (sumsq / count) - (mean * mean);
    double stddev = sqrt(fabs(variance));

    return stddev;
}

// Automatic radius selection based on noise level
static double auto_radius(double stddev)
{
    if (stddev < 10) return 0.5;  // very clean image
    if (stddev < 25) return 1.0;  // slight noise
    if (stddev < 50) return 2.0;  // medium noise
    if (stddev < 80) return 3.0;  // strong noise
    return 4.0;                   // very strong noise
}

// Reading and writing
static MagickWand *read_image(const char *path)
{
    MagickWand *wand = NewMagickWand();
    if (MagickReadImage(wand, path) == MagickFalse)
    {
        fprintf(stderr, "Error: cannot read '%s'\n", path);
        DestroyMagickWand(wand);
        return NULL;
    }
    return wand;
}

static int write_image(MagickWand *wand, const char *path)
{
    if (!wand) return 0;
    if (MagickWriteImage(wand, path) == MagickFalse) return 0;
    return 1;
}

// Main function of automatic noise reduction
static MagickWand *remove_noise(MagickWand *wand)
{
    if (!wand) return NULL;

    // Convert to grayscale for noise estimation
    MagickSetImageType(wand, GrayscaleType);

    // Global noise estimation
    double stddev = compute_gray_stddev(wand);
    double radius = auto_radius(stddev);

    printf(
        "→ Estimated noise = %.2f → Automatic radius = %.2f\n", stddev, radius
    );

    // Apply median filter
    MagickWand *clean = CloneMagickWand(wand);
    if (MagickStatisticImage(clean, MedianStatistic, radius, radius) ==
        MagickFalse)
    {
        fprintf(stderr, "Error: failed to apply median filter.\n");
        DestroyMagickWand(clean);
        return NULL;
    }

    return clean;
}

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

    MagickWand *wand = read_image(input_path);
    if (!wand)
    {
        MagickWandTerminus();
        return 1;
    }

    MagickWand *denoised = remove_noise(wand);
    if (denoised)
    {
        if (write_image(denoised, output_path))
            printf("Denoised image recorded under : %s\n", output_path);
        DestroyMagickWand(denoised);
    }

    DestroyMagickWand(wand);
    MagickWandTerminus();
    return 0;
}
