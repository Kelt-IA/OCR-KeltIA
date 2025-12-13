// demos/custom_letters_training.c - Entrenamiento CNN para dataset custom
// (OVERFITTING)
#include "../include/io/bitmap_loader.h"
#include "../include/nn/accuracy_metrics.h"
#include "../include/nn/network.h"
#include "../include/nn/network_io.h"
#include "../include/nn/train.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EPOCHS 10000

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(
            stderr,
            "Usage: %s <dataset_path> <save_path> [--model <model_path>]\n",
            argv[0]
        );
        fprintf(stderr, "Example: %s dataset/letters models/custom\n", argv[0]);
        fprintf(
            stderr,
            "         %s dataset/letters models/custom --model "
            "models/custom/cnn-epoch-20.nn\n",
            argv[0]
        );
        return 1;
    }

    signal(SIGINT, global_sigint_handler);

    char *dataset_path = argv[1];
    char *save_path = argv[2];
    char *model_path = NULL;

    // Parse arguments
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "--model") == 0 && i + 1 < argc)
        {
            model_path = argv[i + 1];
            i++;
        }
    }

    printf("\n╔════════════════════════════════════════╗\n");
    printf("║  Custom Letter Dataset CNN Training    ║\n");
    printf("║         (OVERFITTING MODE)             ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    // Load custom dataset from folders
    printf("Loading dataset from: %s\n", dataset_path);
    ImageData *image_data = load_dataset_from_folders(dataset_path);

    if (!image_data)
    {
        fprintf(stderr, "Failed to load data from %s\n", dataset_path);
        fprintf(stderr, "Make sure the dataset has A-Z folders with images\n");
        return EXIT_FAILURE;
    }

    printf("  Loaded %zu images (28x28)\n", image_data->num_images);

    // Convert to dataset format
    Dataset *dataset = NULL;
    images_to_dataset(image_data, &dataset, 26);  // 26 letters A-Z

    if (!dataset)
    {
        fprintf(stderr, "Failed to convert to dataset format\n");
        free_image_data(image_data);
        return EXIT_FAILURE;
    }

    int const OUTPUTS = 26;  // A-Z letters
    NeuronalNetwork nn;

    if (!model_path)
    {
        printf("\nCreating new CNN...\n");

        ConvLayer conv_configs[2];

        // Conv1: 1x28x28 -> 8 filters 5x5 -> 8x24x24
        int err1 =
            create_conv_layer(&conv_configs[0], 1, 28, 28, 8, 5, 5, 1, 0);
        if (err1 != 0)
        {
            fprintf(stderr, "Error creating conv layer 1\n");
            free_dataset(dataset);
            free_image_data(image_data);
            return EXIT_FAILURE;
        }

        // Conv2: 8x12x12 -> 16 filters 3x3 -> 16x10x10
        int err2 =
            create_conv_layer(&conv_configs[1], 8, 12, 12, 16, 3, 3, 1, 0);
        if (err2 != 0)
        {
            fprintf(stderr, "Error creating conv layer 2\n");
            free_conv_layer(&conv_configs[0]);
            free_dataset(dataset);
            free_image_data(image_data);
            return EXIT_FAILURE;
        }

        size_t dense_neurons[] = {128, OUTPUTS};
        ActivationType activations[] = {
            ACTIVATION_LEAKY_RELU, ACTIVATION_LEAKY_RELU
        };

        ErrorCode err =
            create_cnn(2, conv_configs, 2, dense_neurons, activations, &nn);
        if (err != NN_ERR_OK)
        {
            fprintf(
                stderr, "Error creating CNN: %s\n", nn_error_to_string(err)
            );
            free_conv_layer(&conv_configs[0]);
            free_conv_layer(&conv_configs[1]);
            free_dataset(dataset);
            free_image_data(image_data);
            return EXIT_FAILURE;
        }
    }
    else
    {
        printf("\nLoading model: %s\n", model_path);
        ErrorCode err = load_nn(model_path, &nn);
        if (err != NN_ERR_OK)
        {
            fprintf(
                stderr, "Error loading model: %s\n", nn_error_to_string(err)
            );
            free_dataset(dataset);
            free_image_data(image_data);
            return EXIT_FAILURE;
        }

        printf("Model loaded successfully:\n");
        printf("  Conv layers: %zu\n", nn.n_conv_layers);
        printf("  Dense layers: %zu\n", nn.n_layers);
    }

    nn.learning_rate = 0.01;
    size_t total_epochs = 0;
    const size_t epochs_per_save = 1;
    const size_t batch_size = 1;

    printf("\nTraining Configuration:\n");
    printf("  Learning rate: %.4f\n", nn.learning_rate);
    printf("  Optimizer: SGD (batch_size=1)\n");
    printf("  Max epochs: %d\n", MAX_EPOCHS);
    printf("  Total samples: %d\n", dataset->num_samples);
    printf("  Mode: OVERFITTING (train = test)\n\n");

    printf("Starting training...\n");
    printf("Press Ctrl+C to stop\n\n");

    time_t start_time = time(NULL);

    while (!stop_requested && total_epochs < MAX_EPOCHS)
    {
        // time_t epoch_start = time(NULL);

        train_nn(&nn, dataset, epochs_per_save, batch_size);
        total_epochs += epochs_per_save;

        // time_t epoch_end = time(NULL);
        // double epoch_time = difftime(epoch_end, epoch_start);

        // Evaluate on same dataset (overfitting check)
        // EvaluationMetrics metrics = evaluate_network(&nn, dataset);
        // printf(
        //     "    Accuracy: %.2f%% (%d/%d)\n",
        //     metrics.accuracy * 100.0, metrics.correct_predictions,
        //     dataset->num_samples
        // );

        // Save every 15 epochs
        if (total_epochs % 15 == 0 && total_epochs > 0)
        {
            printf("=== Epoch %zu/%d ===\n", total_epochs + 1, MAX_EPOCHS);
            // Evaluate on same dataset(overfitting check)
            EvaluationMetrics metrics = evaluate_network(&nn, dataset);
            printf(
                "    Accuracy: %.2f%% (%d/%d)\n", metrics.accuracy * 100.0,
                metrics.correct_predictions, dataset->num_samples
            );

            char filepath[512];
            snprintf(
                filepath, sizeof(filepath), "%s/custom-cnn-epoch-%zu.nn",
                save_path, total_epochs
            );
            ErrorCode save_err = save_nn(filepath, &nn);
            if (save_err == NN_ERR_OK)
            {
                printf("    ✓ Model saved: %s\n", filepath);
            }
            else
            {
                fprintf(
                    stderr, "    ✗ Error saving model: %s\n",
                    nn_error_to_string(save_err)
                );
            }
        }
    }

    time_t end_time = time(NULL);
    double total_time = difftime(end_time, start_time);

    printf("\n╔════════════════════════════════════════╗\n");
    printf("║       Training Complete!              ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("  Total epochs: %zu\n", total_epochs);
    printf(
        "  Total time: %.0f seconds (%.1f minutes)\n", total_time,
        total_time / 60.0
    );

    // Final evaluation
    printf("\nFinal Evaluation:\n");
    EvaluationMetrics final_metrics = evaluate_network(&nn, dataset);
    printf("  Accuracy: %.2f%%\n", final_metrics.accuracy * 100.0);
    printf(
        "  Correct: %d / %d\n", final_metrics.correct_predictions,
        dataset->num_samples
    );
    printf("  MSE: %.6f\n", final_metrics.mse);

    // Save final model
    char final_path[512];
    snprintf(
        final_path, sizeof(final_path), "%s/custom-cnn-final.nn", save_path
    );
    ErrorCode final_err = save_nn(final_path, &nn);
    if (final_err == NN_ERR_OK)
    {
        printf("\nFinal model saved: %s\n", final_path);
    }

    // Cleanup
    free_nn(&nn);
    free_dataset(dataset);
    free_image_data(image_data);

    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
