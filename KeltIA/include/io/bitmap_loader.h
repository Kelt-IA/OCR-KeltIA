// bitmap_loader.h
#pragma once

#include "../nn/accuracy_metrics.h"
#include <MagickWand/MagickWand.h>
#include <stdint.h>
#include <stdio.h>

// Structure to hold image dataset
typedef struct
{
    uint8_t **images;   // Array of grayscale images
    uint8_t *labels;    // Array of labels
    size_t num_images;  // Number of images
    size_t width;       // Image width
    size_t height;      // Image height
} ImageData;

// Load images from directory using MagickWand
ImageData *
load_images_magick(const char *directory_path, const char *label_file);

// Load single image using MagickWand
uint8_t *load_single_image_magick(
    const char *filepath,
    size_t *out_width,
    size_t *out_height
);

// Convert ImageData to Dataset
void images_to_dataset(
    ImageData *images,
    Dataset **out_dataset,
    size_t num_classes
);

// Free image data
void free_image_data(ImageData *data);
