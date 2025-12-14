// demos/custom_letters_training.c - Entrenamiento CNN para dataset custom
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
            "Usage: %s <dataset_path> <save_path> [--model <model_path>] "
            "[--test]\n",
            argv[0]
        );
        fprintf(stderr, "Example: %s dataset/letters models/custom\n", argv[0]);
        fprintf(
            stderr,
            "         %s dataset/letters models/custom --model "
            "models/custom/cnn-epoch-20.nn\n",
            argv[0]
        );
        fprintf(
            stderr,
            "         %s dataset/letters models/custom --test  # Enable "
            "train/test split\n",
            argv[0]
        );
        return 1;
    }

    signal(SIGINT, global_sigint_handler);

    char *dataset_path = argv[1];
    char *save_path = argv[2];
    char *model_path = NULL;
    int use_test_split = 0;

    // Parse arguments
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "--model") == 0 && i + 1 < argc)
        {
            model_path = argv[i + 1];
            i++;
        }
        else if (strcmp(argv[i], "--test") == 0) { use_test_split = 1; }
    }

    printf("\n╔════════════════════════════════════════╗\n");
    printf("║  Custom Letter Dataset CNN Training    ║\n");
    if (use_test_split)
    {
        printf("║       (TRAIN/TEST SPLIT MODE)          ║\n");
    }
    else
    {
        printf("║         (OVERFITTING MODE)             ║\n");
    }
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

    // Split dataset if --test flag is set
    Dataset *train_dataset = NULL;
    Dataset *test_dataset = NULL;
    ImageData *test_image_data = NULL;

    if (use_test_split)
    {
        // Use 10% for testing
        size_t test_size = image_data->num_images / 10;
        size_t train_size = image_data->num_images - test_size;

        printf("\nSplitting dataset:\n");
        printf("  Training: %zu images\n", train_size);
        printf("  Testing: %zu images\n", test_size);

        // Create test ImageData structure (shares pointers with image_data)
        test_image_data = malloc(sizeof(ImageData));
        if (!test_image_data)
        {
            fprintf(stderr, "Error allocating test data\n");
            free_image_data(image_data);
            return EXIT_FAILURE;
        }

        test_image_data->num_images = test_size;
        test_image_data->width = 28;
        test_image_data->height = 28;
        test_image_data->images = &image_data->images[train_size];
        test_image_data->labels = &image_data->labels[train_size];

        // Adjust train size
        image_data->num_images = train_size;

        // Convert both to datasets
        images_to_dataset(image_data, &train_dataset, 26);
        images_to_dataset(test_image_data, &test_dataset, 26);

        if (!train_dataset || !test_dataset)
        {
            fprintf(stderr, "Failed to convert to dataset format\n");
            free(test_image_data);
            free_image_data(image_data);
            return EXIT_FAILURE;
        }
    }
    else
    {
        // Use all data for training (overfitting mode)
        images_to_dataset(image_data, &train_dataset, 26);

        if (!train_dataset)
        {
            fprintf(stderr, "Failed to convert to dataset format\n");
            free_image_data(image_data);
            return EXIT_FAILURE;
        }
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
            free_dataset(train_dataset);
            if (test_dataset) free_dataset(test_dataset);
            if (test_image_data) free(test_image_data);
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
            free_dataset(train_dataset);
            if (test_dataset) free_dataset(test_dataset);
            if (test_image_data) free(test_image_data);
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
            free_dataset(train_dataset);
            if (test_dataset) free_dataset(test_dataset);
            if (test_image_data) free(test_image_data);
            free_image_data(image_data);
            return EXIT_FAILURE;
        }

        printf("\nCNN Architecture:\n");
        printf("  Input: 1x28x28\n");
        printf(
            "  Conv1: 1x28x28 -> 8 filters 5x5 -> 8x24x24 -> MaxPool -> "
            "8x12x12\n"
        );
        printf(
            "  Conv2: 8x12x12 -> 16 filters 3x3 -> 16x10x10 -> MaxPool -> "
            "16x5x5\n"
        );
        printf("  Flatten: %zu (16*5*5 = 400)\n", nn.flattened_size);
        printf("  Dense1: %zu -> 128 (Leaky ReLU)\n", nn.flattened_size);
        printf("  Dense2: 128 -> %d (Leaky ReLU)\n", OUTPUTS);
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
            if (test_dataset) free_dataset(test_dataset);
            if (test_image_data) free(test_image_data);
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
    printf("  Training samples: %d\n", train_dataset->num_samples);
    if (use_test_split)
    {
        printf("  Test samples: %d\n", test_dataset->num_samples);
    }
    else
    {
        printf("  Mode: OVERFITTING (no test set)\n");
    }
    printf("\n");

    printf("Starting training...\n");
    printf("Press Ctrl+C to stop\n\n");

    time_t start_time = time(NULL);

    while (!stop_requested && total_epochs < MAX_EPOCHS)
    {
        train_nn(&nn, train_dataset, epochs_per_save, batch_size);
        total_epochs += epochs_per_save;

        // Evaluate and save every 15 epochs
        if (total_epochs % 15 == 0 && total_epochs > 0)
        {
            printf("=== Epoch %zu/%d ===\n", total_epochs, MAX_EPOCHS);

            // Evaluate on training set
            EvaluationMetrics train_metrics =
                evaluate_network(&nn, train_dataset);
            printf(
                "  Train Accuracy: %.2f%% (%d/%d)\n",
                train_metrics.accuracy * 100.0,
                train_metrics.correct_predictions, train_dataset->num_samples
            );

            // Evaluate on test set if available
            if (use_test_split && test_dataset)
            {
                EvaluationMetrics test_metrics =
                    evaluate_network(&nn, test_dataset);
                printf(
                    "  Test Accuracy: %.2f%% (%d/%d)\n",
                    test_metrics.accuracy * 100.0,
                    test_metrics.correct_predictions, test_dataset->num_samples
                );
                printf("  Test MSE: %.6f\n", test_metrics.mse);
            }

            // Save model
            char filepath[512];
            snprintf(
                filepath, sizeof(filepath), "%s/custom-cnn-epoch-%zu.nn",
                save_path, total_epochs
            );
            ErrorCode save_err = save_nn(filepath, &nn);
            if (save_err == NN_ERR_OK)
            {
                printf("  ✓ Model saved: %s\n\n", filepath);
            }
            else
            {
                fprintf(
                    stderr, "  ✗ Error saving model: %s\n\n",
                    nn_error_to_string(save_err)
                );
            }

            // Stop if perfect accuracy on training set
            if (!use_test_split && train_metrics.accuracy >= 1.0)
            {
                printf("🎉 Perfect accuracy achieved! Stopping training.\n");
                break;
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
    EvaluationMetrics final_train = evaluate_network(&nn, train_dataset);
    printf("  Train Accuracy: %.2f%%\n", final_train.accuracy * 100.0);
    printf(
        "  Train Correct: %d / %d\n", final_train.correct_predictions,
        train_dataset->num_samples
    );

    if (use_test_split && test_dataset)
    {
        EvaluationMetrics final_test = evaluate_network(&nn, test_dataset);
        printf("  Test Accuracy: %.2f%%\n", final_test.accuracy * 100.0);
        printf(
            "  Test Correct: %d / %d\n", final_test.correct_predictions,
            test_dataset->num_samples
        );
        printf("  Test MSE: %.6f\n", final_test.mse);
    }

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
    free_dataset(train_dataset);
    if (test_dataset) free_dataset(test_dataset);
    if (test_image_data) free(test_image_data);
    free_image_data(image_data);

    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
