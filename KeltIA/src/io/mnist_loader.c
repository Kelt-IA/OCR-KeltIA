// src/io/mnist_loader.c
#include "../../include/io/mnist_loader.h"
#include "../../include/nn/include_nn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Read 32-bit big-endian integer
static uint32_t read_int32(FILE *f)
{
    uint8_t bytes[4];
    if (fread(bytes, 1, 4, f) != 4)
    {
        fprintf(stderr, "Error reading 4 bytes\n");
        return 0;
    }
    return (bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3];
}

MNISTData *load_mnist_images(const char *image_path, const char *label_path)
{
    MNISTData *data = malloc(sizeof(MNISTData));
    if (!data)
    {
        fprintf(stderr, "Error allocating MNISTData\n");
        return NULL;
    }
    memset(data, 0, sizeof(MNISTData));

    // ===== LOAD IMAGES =====
    FILE *img_file = fopen(image_path, "rb");
    if (!img_file)
    {
        fprintf(stderr, "Error opening image file: %s\n", image_path);
        free(data);
        return NULL;
    }

    uint32_t magic = read_int32(img_file);
    if (magic != 0x00000803)  // MNIST image magic number
    {
        fprintf(
            stderr,
            "Invalid image file magic number: 0x%08X (expected 0x00000803)\n",
            magic
        );
        fclose(img_file);
        free(data);
        return NULL;
    }

    uint32_t num_images = read_int32(img_file);
    uint32_t num_rows = read_int32(img_file);
    uint32_t num_cols = read_int32(img_file);

    if (num_rows != 28 || num_cols != 28)
    {
        fprintf(
            stderr, "Invalid image dimensions: %ux%u (expected 28x28)\n",
            num_rows, num_cols
        );
        fclose(img_file);
        free(data);
        return NULL;
    }

    printf(
        "Loading MNIST: %u images, %ux%u pixels\n", num_images, num_rows,
        num_cols
    );

    data->num_images = num_images;
    data->images = malloc(num_images * sizeof(uint8_t *));
    if (!data->images)
    {
        fprintf(stderr, "Error allocating images array\n");
        fclose(img_file);
        free(data);
        return NULL;
    }

    size_t image_size = num_rows * num_cols;  // 784
    for (size_t i = 0; i < num_images; i++)
    {
        data->images[i] = malloc(image_size * sizeof(uint8_t));
        if (!data->images[i])
        {
            fprintf(stderr, "Error allocating image %zu\n", i);
            for (size_t j = 0; j < i; j++) free(data->images[j]);
            free(data->images);
            fclose(img_file);
            free(data);
            return NULL;
        }

        if (fread(data->images[i], 1, image_size, img_file) != image_size)
        {
            fprintf(stderr, "Error reading image %zu\n", i);
            for (size_t j = 0; j <= i; j++) free(data->images[j]);
            free(data->images);
            fclose(img_file);
            free(data);
            return NULL;
        }
    }
    fclose(img_file);

    // ===== LOAD LABELS =====
    FILE *lbl_file = fopen(label_path, "rb");
    if (!lbl_file)
    {
        fprintf(stderr, "Error opening label file: %s\n", label_path);
        for (size_t i = 0; i < num_images; i++) free(data->images[i]);
        free(data->images);
        free(data);
        return NULL;
    }

    magic = read_int32(lbl_file);
    if (magic != 0x00000801)  // MNIST label magic number
    {
        fprintf(
            stderr,
            "Invalid label file magic number: 0x%08X (expected 0x00000801)\n",
            magic
        );
        fclose(lbl_file);
        for (size_t i = 0; i < num_images; i++) free(data->images[i]);
        free(data->images);
        free(data);
        return NULL;
    }

    uint32_t num_labels = read_int32(lbl_file);
    if (num_labels != num_images)
    {
        fprintf(
            stderr, "Number of labels (%u) doesn't match images (%u)\n",
            num_labels, num_images
        );
        fclose(lbl_file);
        for (size_t i = 0; i < num_images; i++) free(data->images[i]);
        free(data->images);
        free(data);
        return NULL;
    }

    data->labels = malloc(num_labels * sizeof(uint8_t));
    if (!data->labels)
    {
        fprintf(stderr, "Error allocating labels array\n");
        fclose(lbl_file);
        for (size_t i = 0; i < num_images; i++) free(data->images[i]);
        free(data->images);
        free(data);
        return NULL;
    }

    if (fread(data->labels, 1, num_labels, lbl_file) != num_labels)
    {
        fprintf(stderr, "Error reading labels\n");
        fclose(lbl_file);
        free(data->labels);
        for (size_t i = 0; i < num_images; i++) free(data->images[i]);
        free(data->images);
        free(data);
        return NULL;
    }
    fclose(lbl_file);

    printf(
        "✓ Successfully loaded %zu MNIST images and labels\n", data->num_images
    );
    return data;
}

void free_mnist_data(MNISTData *data)
{
    if (!data) return;

    if (data->images)
    {
        for (size_t i = 0; i < data->num_images; i++)
        {
            if (data->images[i]) free(data->images[i]);
        }
        free(data->images);
    }

    if (data->labels) free(data->labels);
    free(data);
}

void mnist_to_dataset(MNISTData *mnist, Dataset **out_dataset)
{
    Dataset *dataset = malloc(sizeof(Dataset));
    if (!dataset)
    {
        fprintf(stderr, "Error allocating dataset\n");
        return;
    }

    dataset->num_samples = mnist->num_images;
    dataset->input_size = 28 * 28;  // 784
    dataset->output_size = 10;      // 10 digits (0-9)

    dataset->inputs = malloc(dataset->num_samples * sizeof(double *));
    dataset->targets = malloc(dataset->num_samples * sizeof(double *));

    if (!dataset->inputs || !dataset->targets)
    {
        fprintf(stderr, "Error allocating dataset arrays\n");
        free(dataset);
        return;
    }

    for (size_t i = 0; i < (size_t)dataset->num_samples; i++)
    {
        // Normalize pixels from [0, 255] to [0, 1]
        dataset->inputs[i] = malloc(784 * sizeof(double));
        if (!dataset->inputs[i])
        {
            fprintf(stderr, "Error allocating input %zu\n", i);
            for (size_t j = 0; j < i; j++)
            {
                free(dataset->inputs[j]);
                free(dataset->targets[j]);
            }
            free(dataset->inputs);
            free(dataset->targets);
            free(dataset);
            return;
        }

        for (size_t j = 0; j < 784; j++)
        {
            dataset->inputs[i][j] = mnist->images[i][j] / 255.0;
        }

        // One-hot encode labels (0-9 -> [0,0,0,1,0,0,0,0,0,0] for digit 3)
        dataset->targets[i] = calloc(10, sizeof(double));
        if (!dataset->targets[i])
        {
            fprintf(stderr, "Error allocating target %zu\n", i);
            for (size_t j = 0; j <= i; j++) free(dataset->inputs[j]);
            for (size_t j = 0; j < i; j++) free(dataset->targets[j]);
            free(dataset->inputs);
            free(dataset->targets);
            free(dataset);
            return;
        }
        dataset->targets[i][mnist->labels[i]] = 1.0;
    }

    *out_dataset = dataset;
    printf(
        "✓ Converted to dataset: %d samples, input_size=%zu, output_size=%zu\n",
        dataset->num_samples, dataset->input_size, dataset->output_size
    );
}
