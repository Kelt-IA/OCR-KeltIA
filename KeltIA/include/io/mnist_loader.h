// mnist_loader.h
#pragma once

#include "../nn/accuracy_metrics.h"
#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint8_t **images;  // Array of images (each 28x28)
    uint8_t *labels;   // Array of labels (0-9)
    size_t num_images;
} MNISTData;

MNISTData *load_mnist_images(const char *image_path, const char *label_path);
void free_mnist_data(MNISTData *data);
void mnist_to_dataset(MNISTData *mnist, Dataset **out_dataset);
void emnist_letters_to_dataset(MNISTData *mnist, Dataset **out_dataset);
