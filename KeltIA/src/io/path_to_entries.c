// #include "../../include/io/path_to_entries.h"
// #include "../../include/io/parse_csv.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

double *read_image_pixels(const char *image_path, size_t *pixels_count)
{
    MagickWand *wand = NewMagickWand();

    if (MagickReadImage(wand, image_path) == MagickFalse)
    {
        fprintf(stderr, "Error reading image: %s\n", image_path);
        DestroyMagickWand(wand);
        return NULL;
    }

    size_t width = MagickGetImageWidth(wand);
    size_t height = MagickGetImageHeight(wand);
    *pixels_count = width * height;

    int *pixels_int = malloc(*pixels_count * sizeof(int));
    MagickExportImagePixels(
        wand, 0, 0, width, height, "I", CharPixel, pixels_int
    );

    // Convert to doubles [0.0, 1.0]
    double *pixels_double = malloc(*pixels_count * sizeof(double));
    for (size_t i = 0; i < *pixels_count; i++)
    {
        double normalized = pixels_int[i] / 255.0;
        pixels_double[i] = (normalized > 0.5) ? 1.0 : 0.0;
    }

    free(pixels_int);
    DestroyMagickWand(wand);
    return pixels_double;
}

Dataset *csv_to_dataset(CSV *csv_data, const char *base_path)
{
    // Validar columnas
    if (csv_data->num_columns != 2)
    {
        fprintf(stderr, "Error: the CSV must have exactly 2 columns\n");
        return NULL;
    }

    // Verificar que las columnas sean "filename" y "label" en ese orden
    if (strcmp(csv_data->labels[0], "filename") != 0 ||
        strcmp(csv_data->labels[1], "label") != 0)
    {
        fprintf(
            stderr,
            "Error: columns must be 'filename' and 'label' in this order\n"
        );
        fprintf(
            stderr, "columns found: '%s' y '%s'\n", csv_data->labels[0],
            csv_data->labels[1]
        );
        return NULL;
    }

    // Allocar dataset
    Dataset *dataset = malloc(sizeof(Dataset));
    dataset->num_samples = csv_data->entries;
    dataset->output_size = 26;

    char full_path[4096];
    snprintf(
        full_path, sizeof(full_path), "%s/%s", base_path, csv_data->rows[0][0]
    );

    size_t input_size = 0;
    double *test_pixels = read_image_pixels(full_path, &input_size);
    if (!test_pixels)
    {
        fprintf(stderr, "Error reading first image\n");
        free(dataset);
        return NULL;
    }
    free(test_pixels);

    dataset->input_size = input_size;

    dataset->inputs = malloc(dataset->num_samples * sizeof(double *));
    dataset->targets = malloc(dataset->num_samples * sizeof(double *));

    // Process each entrie from CSV
    int valid_samples = 0;
    for (size_t i = 0; i < csv_data->entries; i++)
    {
        char *filename = csv_data->rows[0][i];
        char *label_str = csv_data->rows[1][i];

        // Construir ruta completa
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, filename);

        // Leer píxeles
        size_t pixel_count = 0;
        double *pixels = read_image_pixels(full_path, &pixel_count);

        if (!pixels || pixel_count != input_size)
        {
            fprintf(
                stderr, "Advertencia: error reading %s, skiping...\n", full_path
            );
            continue;
        }

        // Convert label to one-hot
        double *onehot = label_to_onehot(label_str[0]);

        dataset->inputs[valid_samples] = pixels;
        dataset->targets[valid_samples] = onehot;
        valid_samples++;
    }

    dataset->num_samples = valid_samples;

    printf(
        "Dataset loaded: %d entries, input_size=%zu, output_size=%zu\n",
        dataset->num_samples, dataset->input_size, dataset->output_size
    );

    return dataset;
}
