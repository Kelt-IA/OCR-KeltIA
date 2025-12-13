#include "../../include/nn/callback_for_ui.h"
#include "../../include/io/bitmap_loader.h"
#include "../../include/nn/network_io.h"
#include "../../include/nn/train.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Internal training thread function
static void *training_thread_func(void *arg)
{
    TrainingState *state = (TrainingState *)arg;

    // Load dataset
    state->image_data = load_dataset_from_folders(state->config.dataset_folder);
    if (!state->image_data)
    {
        atomic_store(&state->is_training, false);
        return NULL;
    }

    // Convert to dataset format
    images_to_dataset(state->image_data, &state->dataset, 26);  // 26 letters
    if (!state->dataset)
    {
        free_image_data(state->image_data);
        atomic_store(&state->is_training, false);
        return NULL;
    }

    // Load or create CNN
    state->nn = malloc(sizeof(NeuronalNetwork));

    if (state->config.model_path)
    {
        // Load existing model
        ErrorCode err = load_nn(state->config.model_path, state->nn);
        if (err != NN_ERR_OK)
        {
            free(state->nn);
            free_dataset(state->dataset);
            free_image_data(state->image_data);
            atomic_store(&state->is_training, false);
            return NULL;
        }
    }
    else
    {
        // Create new CNN
        ConvLayer conv_configs[2];

        // Conv1: 1x28x28 -> 8 filters 5x5 -> 8x24x24
        int err1 =
            create_conv_layer(&conv_configs[0], 1, 28, 28, 8, 5, 5, 1, 0);
        if (err1 != 0)
        {
            free(state->nn);
            free_dataset(state->dataset);
            free_image_data(state->image_data);
            atomic_store(&state->is_training, false);
            return NULL;
        }

        // Conv2: 8x12x12 -> 16 filters 3x3 -> 16x10x10
        int err2 =
            create_conv_layer(&conv_configs[1], 8, 12, 12, 16, 3, 3, 1, 0);
        if (err2 != 0)
        {
            free_conv_layer(&conv_configs[0]);
            free(state->nn);
            free_dataset(state->dataset);
            free_image_data(state->image_data);
            atomic_store(&state->is_training, false);
            return NULL;
        }

        size_t dense_neurons[] = {128, 26};  // 26 outputs (A-Z)
        ActivationType activations[] = {
            ACTIVATION_LEAKY_RELU, ACTIVATION_LEAKY_RELU
        };

        ErrorCode err = create_cnn(
            2, conv_configs, 2, dense_neurons, activations, state->nn
        );
        if (err != NN_ERR_OK)
        {
            free_conv_layer(&conv_configs[0]);
            free_conv_layer(&conv_configs[1]);
            free(state->nn);
            free_dataset(state->dataset);
            free_image_data(state->image_data);
            atomic_store(&state->is_training, false);
            return NULL;
        }
    }

    state->nn->learning_rate = 0.01;
    const size_t batch_size = 1;  // SGD

    // Training loop
    atomic_store(&state->current_epoch, 0);

    while (!atomic_load(&state->stop_requested) &&
           atomic_load(&state->current_epoch) < state->config.max_epochs)
    {
        int current = atomic_load(&state->current_epoch);

        // Train one epoch
        train_nn(state->nn, state->dataset, 1, batch_size);

        atomic_store(&state->current_epoch, current + 1);
        current = atomic_load(&state->current_epoch);

        // Evaluate and callback to UI
        if (state->config.callback)
        {
            EvaluationMetrics metrics =
                evaluate_network(state->nn, state->dataset);
            state->config.callback(&metrics);
        }

        // Save model at intervals
        if (current % state->config.save_interval == 0 && current > 0)
        {
            char filepath[512];
            snprintf(
                filepath, sizeof(filepath), "%s/cnn-epoch-%d.nn",
                state->config.save_folder, current
            );
            save_nn(filepath, state->nn);
        }

        // Stop if perfect accuracy
        EvaluationMetrics check = evaluate_network(state->nn, state->dataset);
        if (check.accuracy >= 1.0) { break; }
    }

    // Save final model
    char final_path[512];
    snprintf(
        final_path, sizeof(final_path), "%s/cnn-final.nn",
        state->config.save_folder
    );
    save_nn(final_path, state->nn);

    atomic_store(&state->is_training, false);
    return NULL;
}

// Start training in background thread
int start_training(TrainingConfig *config, TrainingState **out_state)
{
    if (!config || !config->dataset_folder || !config->save_folder)
    {
        return -1;
    }

    TrainingState *state = calloc(1, sizeof(TrainingState));
    if (!state) return -1;

    // Copy config
    state->config = *config;
    if (config->model_path)
    {
        state->config.model_path = strdup(config->model_path);
    }
    state->config.dataset_folder = strdup(config->dataset_folder);
    state->config.save_folder = strdup(config->save_folder);

    // Initialize atomic state
    atomic_store(&state->stop_requested, false);
    atomic_store(&state->is_training, true);
    atomic_store(&state->current_epoch, 0);

    // Create training thread
    if (pthread_create(&state->thread, NULL, training_thread_func, state) != 0)
    {
        free(state->config.dataset_folder);
        free(state->config.save_folder);
        if (state->config.model_path) free(state->config.model_path);
        free(state);
        return -1;
    }

    *out_state = state;
    return 0;
}

// Stop training (call from UI button)
void stop_training(TrainingState *state)
{
    if (!state) return;
    atomic_store(&state->stop_requested, true);
}

// Cleanup after training finished
void cleanup_training_state(TrainingState *state)
{
    if (!state) return;

    // Wait for thread to finish
    if (atomic_load(&state->is_training)) { pthread_join(state->thread, NULL); }

    // Free resources
    if (state->nn)
    {
        free_nn(state->nn);
        free(state->nn);
    }
    if (state->dataset) free_dataset(state->dataset);
    if (state->image_data) free_image_data(state->image_data);

    if (state->config.model_path) free(state->config.model_path);
    if (state->config.dataset_folder) free(state->config.dataset_folder);
    if (state->config.save_folder) free(state->config.save_folder);

    free(state);
}
