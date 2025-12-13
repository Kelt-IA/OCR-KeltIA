// demos/predict_letter.c - Predict single letter from image
#include "../include/io/bitmap_loader.h"
#include "../include/nn/network.h"
#include "../include/nn/network_io.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <model_path> <image_path>\n", argv[0]);
        fprintf(
            stderr, "Example: %s models/custom-cnn-final.nn letter.png\n",
            argv[0]
        );
        return EXIT_FAILURE;
    }

    const char *model_path = argv[1];
    const char *image_path = argv[2];

    printf("╔════════════════════════════════════════╗\n");
    printf("║        Letter Prediction CNN          ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    // Load CNN model
    printf("Loading model: %s\n", model_path);
    NeuronalNetwork nn;
    ErrorCode err = load_nn(model_path, &nn);
    if (err != NN_ERR_OK)
    {
        fprintf(stderr, "✗ Error loading model: %s\n", nn_error_to_string(err));
        return EXIT_FAILURE;
    }
    printf("✓ Model loaded\n\n");

    // Load and preprocess image
    printf("Loading image: %s\n", image_path);
    uint8_t *pixels = resize_and_pad_to_28x28(image_path);
    if (!pixels)
    {
        fprintf(stderr, "✗ Error loading image\n");
        free_nn(&nn);
        return EXIT_FAILURE;
    }
    printf("✓ Image loaded and resized to 28x28\n\n");

    // Normalize to [0, 1]
    double *input = malloc(28 * 28 * sizeof(double));
    for (int i = 0; i < 28 * 28; i++) { input[i] = pixels[i] / 255.0; }
    free(pixels);

    // Run CNN
    printf("Running CNN...\n");
    double output[26];
    compute_nn(&nn, input, output);
    printf("✓ Prediction complete\n\n");

    // Find max prediction
    int predicted = 0;
    double max_conf = output[0];
    for (int i = 1; i < 26; i++)
    {
        if (output[i] > max_conf)
        {
            max_conf = output[i];
            predicted = i;
        }
    }

    char predicted_letter = 'A' + predicted;

    // Display results
    printf("╔════════════════════════════════════════╗\n");
    printf("║            PREDICTION                 ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    printf("  Predicted Letter: %c\n", predicted_letter);
    printf("  Confidence: %.2f%%\n\n", max_conf * 100);

    // Show top 3 predictions
    printf("Top 3 predictions:\n");
    for (int n = 0; n < 3; n++)
    {
        int max_idx = 0;
        double max_val = output[0];

        for (int i = 1; i < 26; i++)
        {
            if (output[i] > max_val)
            {
                max_val = output[i];
                max_idx = i;
            }
        }

        printf("  %d. %c: %.2f%%\n", n + 1, 'A' + max_idx, max_val * 100);
        output[max_idx] = 0;  // Zero out for next iteration
    }
    printf("\n");

    // Cleanup
    free(input);
    free_nn(&nn);

    return EXIT_SUCCESS;
}
