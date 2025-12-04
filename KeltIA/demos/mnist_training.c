// demos/mnist_training.c - Entrenamiento CNN para MNIST
#include "../include/io/mnist_loader.h"
#include "../include/nn/accuracy_metrics.h"
#include "../include/nn/network.h"
#include "../include/nn/network_io.h"
#include "../include/nn/train.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_EPOCHS 50

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(
            stderr, "Usage: %s <save_directory> [--model <path>]\n", argv[0]
        );
        fprintf(stderr, "Example: %s models/mnist\n", argv[0]);
        fprintf(
            stderr,
            "         %s models/mnist --model "
            "models/mnist/mnist-cnn-epoch-5.nn\n",
            argv[0]
        );
        return 1;
    }

    signal(SIGINT, global_sigint_handler);

    char *save_path = argv[1];
    char *model_path = NULL;

    // Parse arguments
    for (int i = 2; i < argc; i++)
    {
        if (strcmp(argv[i], "--model") == 0 && i + 1 < argc)
        {
            model_path = argv[i + 1];
            i++;
        }
    }

    printf("╔════════════════════════════════════════╗\n");
    printf("║      MNIST CNN Training                ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    // Load MNIST data
    printf("Loading MNIST dataset...\n");
    MNISTData *train_data = load_mnist_images(
        "data/mnist/train-images-idx3-ubyte",
        "data/mnist/train-labels-idx1-ubyte"
    );

    if (!train_data)
    {
        fprintf(stderr, "Failed to load training data\n");
        fprintf(stderr, "Make sure files exist in data/mnist/\n");
        return EXIT_FAILURE;
    }

    MNISTData *test_data = load_mnist_images(
        "data/mnist/t10k-images-idx3-ubyte", "data/mnist/t10k-labels-idx1-ubyte"
    );

    if (!test_data)
    {
        fprintf(stderr, "Failed to load test data\n");
        free_mnist_data(train_data);
        return EXIT_FAILURE;
    }

    // Convert to dataset format
    Dataset *train_dataset = NULL;
    Dataset *test_dataset = NULL;

    mnist_to_dataset(train_data, &train_dataset);
    mnist_to_dataset(test_data, &test_dataset);

    if (!train_dataset || !test_dataset)
    {
        fprintf(stderr, "Failed to convert MNIST to dataset\n");
        free_mnist_data(train_data);
        free_mnist_data(test_data);
        return EXIT_FAILURE;
    }

    NeuronalNetwork nn;

    if (!model_path)
    {
        printf("\nCreating new CNN for MNIST...\n");

        // CNN Architecture for MNIST (optimized):
        // Input: 1x28x28
        // Conv1: 1x28x28 -> 8 filters 5x5, stride 1, no padding -> 8x24x24
        // Pool1: 8x24x24 -> 2x2, stride 2 -> 8x12x12
        // Conv2: 8x12x12 -> 16 filters 3x3, stride 1, no padding -> 16x10x10
        // Pool2: 16x10x10 -> 2x2, stride 2 -> 16x5x5 = 400
        // Dense1: 400 -> 128
        // Dense2: 128 -> 10

        ConvLayer conv_configs[2];

        // Conv1: 1x28x28 -> 8 filters 5x5 -> 8x24x24
        int err1 =
            create_conv_layer(&conv_configs[0], 1, 28, 28, 8, 5, 5, 1, 0);
        if (err1 != 0)
        {
            fprintf(stderr, "Error creating conv layer 1\n");
            free_dataset(train_dataset);
            free_dataset(test_dataset);
            free_mnist_data(train_data);
            free_mnist_data(test_data);
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
            free_dataset(test_dataset);
            free_mnist_data(train_data);
            free_mnist_data(test_data);
            return EXIT_FAILURE;
        }

        size_t dense_neurons[] = {128, 10};
        ActivationType activations[] = {
            ACTIVATION_LEAKY_RELU, ACTIVATION_SIGMOID
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
            free_dataset(test_dataset);
            free_mnist_data(train_data);
            free_mnist_data(test_data);
            return EXIT_FAILURE;
        }

        printf("\nCNN Architecture:\n");
        printf("  Input: 1x28x28\n");
        printf("  Conv1: 1x28x28 -> 8 filters 5x5 -> 8x24x24\n");
        printf("  Pool1: 8x24x24 -> 2x2 pool -> 8x12x12\n");
        printf("  Conv2: 8x12x12 -> 16 filters 3x3 -> 16x10x10\n");
        printf("  Pool2: 16x10x10 -> 2x2 pool -> 16x5x5\n");
        printf("  Flatten: %zu (16*5*5 = 400)\n", nn.flattened_size);
        printf("  Dense1: %zu -> 128 (Leaky ReLU)\n", nn.flattened_size);
        printf("  Dense2: 128 -> 10 (Sigmoid)\n");
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
            free_dataset(test_dataset);
            free_mnist_data(train_data);
            free_mnist_data(test_data);
            return EXIT_FAILURE;
        }

        printf("Model loaded successfully:\n");
        printf("  Conv layers: %zu\n", nn.n_conv_layers);
        printf("  Dense layers: %zu\n", nn.n_layers);
    }

    nn.learning_rate = 0.01;
    size_t total_epochs = 0;
    const size_t epochs_per_save = 1;
    const size_t batch_size =
        1;  // SGD (ignored in new train.c, but kept for compatibility)

    printf("\nTraining Configuration:\n");
    printf("  Learning rate: %.4f\n", nn.learning_rate);
    printf("  Optimizer: SGD (batch_size=1)\n");
    printf("  Max epochs: %d\n", MAX_EPOCHS);
    printf("  Training samples: %d\n", train_dataset->num_samples);
    printf("  Test samples: %d\n\n", test_dataset->num_samples);

    printf("Starting training...\n");
    printf("Press Ctrl+C to stop\n\n");

    time_t start_time = time(NULL);

    while (!stop_requested && total_epochs < MAX_EPOCHS)
    {
        time_t epoch_start = time(NULL);

        printf("=== Epoch %zu/%d ===\n", total_epochs + 1, MAX_EPOCHS);

        train_nn(&nn, train_dataset, epochs_per_save, batch_size);
        total_epochs += epochs_per_save;

        time_t epoch_end = time(NULL);
        double epoch_time = difftime(epoch_end, epoch_start);

        // Evaluate on TRAIN set (small sample for speed)
        printf("  Evaluating on training set (first 1000 samples)...\n");
        Dataset train_sample;
        train_sample.num_samples = 1000;
        train_sample.input_size = train_dataset->input_size;
        train_sample.output_size = train_dataset->output_size;
        train_sample.inputs = train_dataset->inputs;
        train_sample.targets = train_dataset->targets;

        EvaluationMetrics train_metrics = evaluate_network(&nn, &train_sample);

        // Evaluate on TEST set
        printf("  Evaluating on test set...\n");
        EvaluationMetrics test_metrics = evaluate_network(&nn, test_dataset);

        printf("\n  Results:\n");
        printf(
            "    Train Accuracy: %.2f%% (%d/1000)\n",
            train_metrics.accuracy * 100.0, train_metrics.correct_predictions
        );
        printf(
            "    Test Accuracy:  %.2f%% (%d/%d)\n",
            test_metrics.accuracy * 100.0, test_metrics.correct_predictions,
            test_dataset->num_samples
        );
        printf("    Test MSE: %.6f\n", test_metrics.mse);
        printf(
            "    Epoch time: %.0f seconds (%.1f minutes)\n", epoch_time,
            epoch_time / 60.0
        );
        printf(
            "    Avg time/sample: %.2f ms\n",
            (epoch_time / train_dataset->num_samples) * 1000.0
        );

        // Save model
        char filepath[512];
        snprintf(
            filepath, sizeof(filepath), "%s/mnist-cnn-epoch-%zu.nn", save_path,
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

        // Learning rate decay
        if (total_epochs % 10 == 0 && total_epochs > 0)
        {
            nn.learning_rate *= 0.9;
            printf("  Learning rate reduced to: %.6f\n\n", nn.learning_rate);
        }

        // Early stopping
        if (test_metrics.accuracy > 0.98)
        {
            printf("🎉 Reached 98%% test accuracy! Stopping.\n");
            break;
        }
    }

    time_t end_time = time(NULL);
    double total_time = difftime(end_time, start_time);

    printf("\n╔════════════════════════════════════════╗\n");
    printf("║         Training Complete!             ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("  Total epochs: %zu\n", total_epochs);
    printf(
        "  Total time: %.0f seconds (%.1f minutes)\n", total_time,
        total_time / 60.0
    );

    // Final evaluation
    printf("\nFinal Test Evaluation:\n");
    EvaluationMetrics final_metrics = evaluate_network(&nn, test_dataset);
    printf("  Accuracy: %.2f%%\n", final_metrics.accuracy * 100.0);
    printf(
        "  Correct: %d / %d\n", final_metrics.correct_predictions,
        test_dataset->num_samples
    );
    printf("  MSE: %.6f\n", final_metrics.mse);

    // Save final model
    char final_path[512];
    snprintf(
        final_path, sizeof(final_path), "%s/mnist-cnn-final.nn", save_path
    );
    ErrorCode final_err = save_nn(final_path, &nn);
    if (final_err == NN_ERR_OK)
    {
        printf("\nFinal model saved: %s\n", final_path);
    }

    // Cleanup
    free_nn(&nn);
    free_dataset(train_dataset);
    free_dataset(test_dataset);
    free_mnist_data(train_data);
    free_mnist_data(test_data);

    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
