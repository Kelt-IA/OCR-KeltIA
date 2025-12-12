#include "../include/io/image_classifier.h"
#include "../include/nn/network.h"
#include "../include/nn/network_io.h"
#include <MagickWand/MagickWand.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <model_path> <images_folder>\n", argv[0]);
        fprintf(
            stderr,
            "Example: %s models/mnist/mnist-cnn-final.nn ./extracted_letters\n",
            argv[0]
        );
        fprintf(
            stderr, "\nClassified images will be saved to: ./classified/\n"
        );
        return EXIT_FAILURE;
    }

    const char *model_path = argv[1];
    const char *input_dir = argv[2];
    const char *output_dir = "./classified";

    printf("╔════════════════════════════════════════╗\n");
    printf("║   Image Classification Organizer     ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    // Load trained CNN model
    printf("Loading model: %s\n", model_path);
    NeuronalNetwork nn;
    ErrorCode err = load_nn(model_path, &nn);

    if (err != NN_ERR_OK)
    {
        fprintf(
            stderr, "✗ Failed to load model: %s\n", nn_error_to_string(err)
        );
        return EXIT_FAILURE;
    }

    printf("✓ Model loaded successfully\n");
    printf("  Conv layers: %zu\n", nn.n_conv_layers);
    printf("  Dense layers: %zu\n", nn.n_layers);

    // Get output classes
    size_t output_classes = nn.layers[nn.n_layers - 1].n_neurons;
    printf("  Output classes: %zu ", output_classes);

    if (output_classes == 26) { printf("(A-Z letters)\n"); }
    else if (output_classes == 10) { printf("(0-9 digits)\n"); }
    else
    {
        printf("(custom)\n");
    }

    printf("\nInput directory: %s\n", input_dir);
    printf("Output directory: %s\n", output_dir);

    // Initialize MagickWand
    MagickWandGenesis();

    // Classify and organize images (28x28 for EMNIST)
    printf("\n");
    int success =
        classify_and_organize_images(&nn, input_dir, output_dir, 28, 28);

    // Cleanup
    MagickWandTerminus();
    free_nn(&nn);

    if (success)
    {
        printf("\n╔════════════════════════════════════════╗\n");
        printf("║   Classification Complete!            ║\n");
        printf("╚════════════════════════════════════════╝\n");
        printf("\nCheck your organized images in: %s/\n", output_dir);
        return EXIT_SUCCESS;
    }
    else
    {
        fprintf(stderr, "\n✗ Classification failed\n");
        return EXIT_FAILURE;
    }
}
