#pragma once

#include "../nn/network.h"
#include <stdint.h>
#include <stdio.h>

// Structure for single image prediction
typedef struct
{
    char *filepath;
    uint8_t *pixels;
    size_t width;
    size_t height;
    int predicted_class;
    double confidence;
} ImagePrediction;

// Create folder structure A-Z
int create_letter_folders(const char *output_dir);

// Classify and organize images
int classify_and_organize_images(
    NeuronalNetwork *nn,
    const char *input_dir,
    const char *output_dir,
    size_t image_width,
    size_t image_height
);

// Load single bitmap and predict
ImagePrediction *predict_single_image(
    NeuronalNetwork *nn,
    const char *filepath,
    size_t width,
    size_t height
);

int is_image_file(const char *filename);

// Free prediction
void free_prediction(ImagePrediction *pred);
