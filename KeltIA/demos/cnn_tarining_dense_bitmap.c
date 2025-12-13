// demos/custom_letters_training.c - Dense NN training for 30x30 images
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

#define MAX_EPOCHS 1000

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(
            stderr,
            "Usage: %s <dataset_path> <save_path> [--model <model_path>]\n",
            argv[0]
        );
        fprintf(stderr, "Example: %s dataset/train models/custom\n", argv[0]);
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

    printf("╔════════════════════════════════════════╗\n");
    printf("║  Custom Letter Dataset NN Training     ║\n");
    printf("║        DENSE NETWORK 30x30             ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    // Load FULL dataset
    printf("Loading FULL dataset...\n");
    ImageData *train_data = load_dataset_from_folders(dataset_path);

    if (!train_data)
    {
        fprintf(stderr, "Failed to load training data from %s\n", dataset_path);
        fprintf(stderr, "Make sure the dataset has A-Z folders with images\n");
        return EXIT_FAILURE;
    }

    printf(
        "\n✓ Training with ALL %zu images (no test split)\n\n",
        train_data->num_images
    );

    // Convert to dataset format
    Dataset *train_dataset = NULL;
    images_to_dataset(train_data, &train_dataset, 26);  // 26 letters A-Z

    if (!train_dataset)
    {
        fprintf(stderr, "Failed to convert to dataset format\n");
        free_image_data(train_data);
        return EXIT_FAILURE;
    }

    int const OUTPUTS = 26;  // A-Z letters
    NeuronalNetwork nn;

    if (!model_path)
    {
        printf("Creating new Dense NN...\n");

        // Dense Architecture:
        // Input: 30x30 = 900 neurons
        // Layer1: 900 -> 64
        // Layer2: 64 -> 26

        size_t layer_sizes[] = {900, 64, OUTPUTS};
        ActivationType activations[3] = {
            ACTIVATION_SIGMOID,  // Hidden layer
            ACTIVATION_SIGMOID,  // Output layer
            ACTIVATION_SOFTMAX
        };

        ErrorCode err = create_cnn(0, NULL, 3, layer_sizes, activations, &nn);
        if (err != NN_ERR_OK)
        {
            fprintf(stderr, "Error creating NN: %s\n", nn_error_to_string(err));
            free_dataset(train_dataset);
            free_image_data(train_data);
            return EXIT_FAILURE;
        }

        printf("\nDense NN Architecture:\n");
        printf("  Input: 30x30 = 900 neurons\n");
        printf("  Layer1: 900 -> 64 (Leaky ReLU)\n");
        printf("  Layer2: 64 -> 26 (Sigmoid)\n");
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
            free_dataset(train_dataset);
            free_image_data(train_data);
            return EXIT_FAILURE;
        }

        printf("Model loaded successfully:\n");
        printf("  Layers: %zu\n", nn.n_layers);
    }

    nn.learning_rate = 0.01;
    size_t total_epochs = 0;
    const size_t epochs_per_save = 10;  // Save every 10 epochs
    const size_t batch_size = 1;

    printf("\nTraining Configuration:\n");
    printf("  Learning rate: %.4f\n", nn.learning_rate);
    printf("  Optimizer: SGD (batch_size=1)\n");
    printf("  Max epochs: %d\n", MAX_EPOCHS);
    printf("  Epochs per checkpoint: %zu\n", epochs_per_save);
    printf("  Training samples: %d\n\n", train_dataset->num_samples);

    printf("Starting training...\n");
    printf("Press Ctrl+C to stop\n\n");

    time_t start_time = time(NULL);
    double best_accuracy = 0.0;
    int epochs_without_improvement = 0;

    while (!stop_requested && total_epochs < MAX_EPOCHS)
    {
        time_t epoch_start = time(NULL);

        // Train for epochs_per_save epochs
        train_nn(&nn, train_dataset, epochs_per_save, batch_size);
        total_epochs += epochs_per_save;

        time_t epoch_end = time(NULL);
        double epoch_time = difftime(epoch_end, epoch_start);

        // Print progress header
        printf("=== Epoch %zu/%d ===\n", total_epochs, MAX_EPOCHS);

        // Evaluate on training set
        printf("  Evaluating...\n");
        EvaluationMetrics metrics = evaluate_network(&nn, train_dataset);

        printf("\n  Results:\n");
        printf(
            "    Accuracy: %.2f%% (%d/%d)\n", metrics.accuracy * 100.0,
            metrics.correct_predictions, train_dataset->num_samples
        );
        printf("    MSE: %.6f\n", metrics.mse);
        printf(
            "    Time for %zu epochs: %.0f seconds (%.1f minutes)\n",
            epochs_per_save, epoch_time, epoch_time / 60.0
        );
        printf(
            "    Avg time/epoch: %.2f seconds\n", epoch_time / epochs_per_save
        );

        // Track best accuracy
        if (metrics.accuracy > best_accuracy)
        {
            best_accuracy = metrics.accuracy;
            epochs_without_improvement = 0;
            printf("    ✓ New best!\n");
        }
        else
        {
            epochs_without_improvement += epochs_per_save;
        }

        // Save model
        char filepath[512];
        snprintf(
            filepath, sizeof(filepath), "%s/custom-nn-epoch-%zu.nn", save_path,
            total_epochs
        );
        ErrorCode save_err = save_nn(filepath, &nn);
        if (save_err == NN_ERR_OK)
        {
            printf("    Model saved: %s\n", filepath);
        }
        else
        {
            fprintf(
                stderr, "    Error saving model: %s\n",
                nn_error_to_string(save_err)
            );
        }

        printf("\n");

        // Learning rate schedule with plateau detection
        if (epochs_without_improvement >= 50)
        {
            nn.learning_rate *= 1.5;
            if (nn.learning_rate > 0.05) nn.learning_rate = 0.05;
            printf(
                "  ⚠️  Plateau detected! LR increased to: %.6f\n\n",
                nn.learning_rate
            );
            epochs_without_improvement = 0;
        }
        else if (total_epochs % 100 == 0 && total_epochs > 0)
        {
            nn.learning_rate *= 0.9;
            printf("  Learning rate reduced to: %.6f\n\n", nn.learning_rate);
        }

        // Early stopping at 99%
        if (metrics.accuracy > 0.99)
        {
            printf("🎉 Reached 99%% accuracy! Stopping.\n");
            break;
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
    printf("  Best accuracy: %.2f%%\n", best_accuracy * 100.0);

    // Save final model
    char final_path[512];
    snprintf(
        final_path, sizeof(final_path), "%s/custom-nn-final.nn", save_path
    );
    ErrorCode final_err = save_nn(final_path, &nn);
    if (final_err == NN_ERR_OK)
    {
        printf("\nFinal model saved: %s\n", final_path);
    }

    // Cleanup
    free_nn(&nn);
    free_dataset(train_dataset);
    free_image_data(train_data);

    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
