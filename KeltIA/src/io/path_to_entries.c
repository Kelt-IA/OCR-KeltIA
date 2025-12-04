#include "../../include/io/parse_csv.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/nn/accuracy_metrics.h"

// (A - Z) -> (0 - 25)
double *label_to_onehot(char label)
{
    double *onehot = calloc(26, sizeof(double));

    if (label >= 'A' && label <= 'Z')
    {
        int index = label - 'A';
        onehot[index] = 1.0;
    }

    return onehot;
}

double *read_image_pixels(
    const char *image_path,
    size_t *pixels_count,
    size_t target_width,
    size_t target_height
)
{
    MagickWand *wand = NewMagickWand();

    if (MagickReadImage(wand, image_path) == MagickFalse)
    {
        fprintf(stderr, "Error reading image: %s\n", image_path);
        DestroyMagickWand(wand);
        return NULL;
    }

    // Resize image to target dimensions
    MagickResizeImage(wand, target_width, target_height, LanczosFilter);

    size_t width = MagickGetImageWidth(wand);
    size_t height = MagickGetImageHeight(wand);
    *pixels_count = width * height;

    unsigned char *pixels_char = malloc(*pixels_count * sizeof(unsigned char));
    MagickExportImagePixels(
        wand, 0, 0, width, height, "I", CharPixel, pixels_char
    );

    // Convert to doubles [0.0, 1.0]
    double *pixels_double = malloc(*pixels_count * sizeof(double));
    for (size_t i = 0; i < *pixels_count; i++)
    {
        double normalized = pixels_char[i] / 255.0;
        pixels_double[i] = (normalized > 0.5) ? 1.0 : 0.0;
    }

    free(pixels_char);
    DestroyMagickWand(wand);
    return pixels_double;
}

Dataset *csv_to_dataset(
    CSV *csv_data,
    const char *base_path,
    size_t target_width,
    size_t target_height
)
{
    // ... validaciones igual ...

    Dataset *dataset = malloc(sizeof(Dataset));
    dataset->num_samples = csv_data->entries;
    dataset->output_size = 26;

    // Input size is now the target size
    dataset->input_size = target_width * target_height;

    char full_path[4096];
    dataset->inputs = malloc(dataset->num_samples * sizeof(double *));
    dataset->targets = malloc(dataset->num_samples * sizeof(double *));

    int valid_samples = 0;
    for (size_t i = 0; i < csv_data->entries; i++)
    {
        char *filename = csv_data->rows[0][i];
        char *label_str = csv_data->rows[1][i];

        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, filename);

        size_t pixel_count = 0;
        double *pixels = read_image_pixels(
            full_path, &pixel_count, target_width, target_height
        );

        if (!pixels || pixel_count != dataset->input_size)
        {
            fprintf(
                stderr, "Warning: error reading %s, skipping...\n", full_path
            );
            continue;
        }

        double *onehot = label_to_onehot(label_str[0]);

        dataset->inputs[valid_samples] = pixels;
        dataset->targets[valid_samples] = onehot;
        valid_samples++;
    }

    dataset->num_samples = valid_samples;

    printf(
        "Dataset loaded: %d entries, input_size=%zu (%zux%zu), "
        "output_size=%zu\n",
        dataset->num_samples, dataset->input_size, target_width, target_height,
        dataset->output_size
    );

    return dataset;
}
