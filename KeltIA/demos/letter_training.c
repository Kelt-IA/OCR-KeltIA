// letter_training.c - CNN training for letter recognition with pooling
#include "../include/io/path_to_entries.h"
#include "../include/nn/accuracy_metrics.h"
#include "../include/nn/network.h"
#include "../include/nn/network_io.h"
#include "../include/nn/train.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EPOCHS 10000
#define TARGET_WIDTH 32
#define TARGET_HEIGHT 32

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        fprintf(
            stderr,
            "Usage: %s <dataset_path> <save_directory_for_models> <log_path> "
            "[--model <path/to/model>]\n",
            argv[0]
        );
        fprintf(stderr, "Expected structure of <dataset_path>:\n");
        fprintf(stderr, "  <dataset_path>/train.csv\n");
        fprintf(stderr, "  <dataset_path>/images/\n");
        return 1;
    }

    signal(SIGINT, global_sigint_handler);

    char *dataset_path = argv[1];
    char *save_path = argv[2];
    char *save_path2 = argv[3];
    char *model_path = NULL;

    for (int i = 4; i < argc; i++)
    {
        if (strcmp(argv[i], "--model") == 0)
        {
            if (i + 1 < argc)
            {
                model_path = argv[i + 1];
                i++;
            }
            else
            {
                fprintf(stderr, "Error: --model requires a path\n");
                return 1;
            }
        }
    }

    // Build file paths
    char csv_path[4096];
    char images_path[4096];

    snprintf(csv_path, sizeof(csv_path), "%s/train.csv", dataset_path);
    snprintf(images_path, sizeof(images_path), "%s/images", dataset_path);

    // Parse CSV file
    CSV *csv = read_csv(csv_path, ",");
    if (!csv)
    {
        fprintf(stderr, "Error reading %s\n", csv_path);
        return EXIT_FAILURE;
    }

    printf(
        "Loading dataset with target size %dx%d...\n", TARGET_WIDTH,
        TARGET_HEIGHT
    );

    Dataset *dataset =
        csv_to_dataset(csv, images_path, TARGET_WIDTH, TARGET_HEIGHT);
    if (!dataset)
    {
        fprintf(stderr, "Error creating dataset\n");
        free_csv(csv);
        return EXIT_FAILURE;
    }

    printf("\nDataset loaded successfully:\n");
    printf("  Samples: %d\n", dataset->num_samples);
    printf(
        "  Input size: %zu (%dx%d images)\n", dataset->input_size, TARGET_WIDTH,
        TARGET_HEIGHT
    );
    printf("  Output size: %zu (26 letters)\n", dataset->output_size);

    NeuronalNetwork nn;

    if (!model_path)
    {
        printf("\nCreating new CNN with MaxPooling...\n");

        ConvLayer conv_configs[2];

        // IMPORTANTE: Inicializa COMPLETAMENTE cada ConvLayer
        // First conv layer: 1x32x32 -> 16 filters 3x3 -> 16x32x32
        int err1 =
            create_conv_layer(&conv_configs[0], 1, 32, 32, 16, 3, 3, 1, 1);
        if (err1 != 0)
        {
            fprintf(stderr, "Error creating conv layer 1\n");
            free_dataset(dataset);
            free_csv(csv);
            return EXIT_FAILURE;
        }

        // Second conv layer: 16x16x16 -> 32 filters 3x3 -> 32x16x16
        int err2 =
            create_conv_layer(&conv_configs[1], 16, 16, 16, 32, 3, 3, 1, 1);
        if (err2 != 0)
        {
            fprintf(stderr, "Error creating conv layer 2\n");
            free_conv_layer(&conv_configs[0]);
            free_dataset(dataset);
            free_csv(csv);
            return EXIT_FAILURE;
        }

        // Dense layers: 2048 -> 128 -> 26
        size_t dense_neurons[] = {128, 26};
        ActivationType activations[] = {
            ACTIVATION_LEAKY_RELU, ACTIVATION_SIGMOID
        };

        // Create CNN with pooling
        ErrorCode err =
            create_cnn(2, conv_configs, 1, 2, dense_neurons, activations, &nn);
        if (err != NN_ERR_OK)
        {
            fprintf(
                stderr, "Error creating CNN: %s\n", nn_error_to_string(err)
            );
            free_conv_layer(&conv_configs[0]);
            free_conv_layer(&conv_configs[1]);
            free_dataset(dataset);
            free_csv(csv);
            return EXIT_FAILURE;
        }

        printf("\nCNN architecture:\n");
        printf(
            "  Conv layer 1: 1x32x32 -> 16 filters 3x3 (padding 1) -> "
            "16x32x32\n"
        );
        printf(
            "  Pool layer 1: 16x32x32 -> 2x2 MaxPool (stride 2) -> 16x16x16\n"
        );
        printf(
            "  Conv layer 2: 16x16x16 -> 32 filters 3x3 (padding 1) -> "
            "32x16x16\n"
        );
        printf(
            "  Pool layer 2: 32x16x16 -> 2x2 MaxPool (stride 2) -> 32x8x8\n"
        );
        printf(
            "  Flatten: %zu values (32 * 8 * 8 = 2048)\n", nn.flattened_size
        );
        printf(
            "  Dense layer 1: %zu -> 128 neurons (Leaky ReLU)\n",
            nn.flattened_size
        );
        printf("  Dense layer 2: 128 -> 26 neurons (Sigmoid)\n");
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
            free_csv(csv);
            return EXIT_FAILURE;
        }

        printf("Model loaded successfully:\n");
        printf("  Conv layers: %zu\n", nn.n_conv_layers);
        printf("  Pool layers: %zu\n", nn.n_pool_layers);
        printf("  Dense layers: %zu\n", nn.n_layers);
        printf("  Flattened size: %zu\n", nn.flattened_size);
    }

    // Training parameters
    nn.learning_rate = 0.001;  // Higher learning rate for faster convergence
    size_t total_epochs = 0;
    const size_t epochs = 100;
    const size_t batch_size = 32;

    printf("\nTraining configuration:\n");
    printf("  Learning rate: %f\n", nn.learning_rate);
    printf("  Batch size: %zu\n", batch_size);
    printf("  Epochs per checkpoint: %zu\n", epochs);
    printf("  Max epochs: %d\n", MAX_EPOCHS);
    printf("  Total samples: %d\n", dataset->num_samples);
    printf(
        "  Batches per epoch: %zu\n\n",
        (dataset->num_samples + batch_size - 1) / batch_size
    );

    printf("Starting training...\n");
    printf("Press Ctrl+C to stop training and save current model.\n\n");

    while (!stop_requested && total_epochs < MAX_EPOCHS)
    {
        printf("=== Epoch %zu/%d ===\n", total_epochs, MAX_EPOCHS);

        train_nn(&nn, dataset, epochs, batch_size);
        total_epochs += epochs;

        // Save model checkpoint
        char filepath[1048];
        char log_path[1048];

        snprintf(
            filepath, sizeof(filepath), "%s/cnn-letter-%zu.nn", save_path,
            total_epochs
        );
        snprintf(
            log_path, sizeof(log_path), "%s/cnn-training-%zu.log", save_path2,
            total_epochs
        );

        // Evaluate network
        printf("Evaluating network...\n");
        EvaluationMetrics metrics = evaluate_network(&nn, dataset);
        log_metrics(log_path, total_epochs, metrics);

        // Save model
        ErrorCode err = save_nn(filepath, &nn);
        if (err != NN_ERR_OK)
        {
            fprintf(
                stderr, "Error saving model: %s\n", nn_error_to_string(err)
            );
        }
        else
        {
            printf("Model saved: %s\n", filepath);
        }

        // Print metrics
        printf("Results:\n");
        printf(
            "  Accuracy: %.4f%% (%d/%d correct)\n", metrics.accuracy * 100.0,
            metrics.correct_predictions, dataset->num_samples
        );
        printf("  MSE: %.6f\n", metrics.mse);
        printf("  Log saved: %s\n\n", log_path);

        // Learning rate decay
        if (total_epochs % 100 == 0 && total_epochs > 0)
        {
            nn.learning_rate *= 0.95;
            printf("Learning rate reduced to: %f\n\n", nn.learning_rate);
        }

        // Early stopping if accuracy is very high
        if (metrics.accuracy > 0.99)
        {
            printf("Reached 99%% accuracy! Stopping training.\n");
            break;
        }
    }

    if (stop_requested) { printf("\n=== Training interrupted by user ===\n"); }
    else if (total_epochs >= MAX_EPOCHS)
    {
        printf("\n=== Training completed: reached maximum epochs ===\n");
    }
    else
    {
        printf("\n=== Training completed: early stopping ===\n");
    }

    // Final evaluation
    printf("\nFinal evaluation:\n");
    EvaluationMetrics final_metrics = evaluate_network(&nn, dataset);
    printf("  Final accuracy: %.4f%%\n", final_metrics.accuracy * 100.0);
    printf("  Final MSE: %.6f\n", final_metrics.mse);
    printf(
        "  Correct predictions: %d/%d\n", final_metrics.correct_predictions,
        dataset->num_samples
    );

    // Save final model
    char final_path[1048];
    snprintf(
        final_path, sizeof(final_path), "%s/cnn-letter-final.nn", save_path
    );
    ErrorCode final_err = save_nn(final_path, &nn);
    if (final_err == NN_ERR_OK)
    {
        printf("\nFinal model saved: %s\n", final_path);
    }

    // Cleanup
    free_nn(&nn);
    free_dataset(dataset);
    free_csv(csv);

    printf("\nTraining session completed.\n");
    return EXIT_SUCCESS;
}
