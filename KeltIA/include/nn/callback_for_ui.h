#pragma once

#include "../io/bitmap_loader.h"
#include "accuracy_metrics.h"
#include "network.h"
#include <pthread.h>
#include <stdatomic.h>

// Training configuration struct
typedef struct
{
    char *model_path;      // NULL = create new, otherwise load existing
    char *dataset_folder;  // Path to dataset folder (A-Z subfolders)
    char *save_folder;     // Where to save models
    int max_epochs;        // Maximum epochs to train
    int save_interval;     // Save model every N epochs
    void (*callback)(EvaluationMetrics *metrics);  // UI callback
} TrainingConfig;

// Training state (thread-safe)
typedef struct
{
    atomic_bool stop_requested;
    atomic_bool is_training;
    atomic_int current_epoch;
    pthread_t thread;
    NeuronalNetwork *nn;
    Dataset *dataset;
    ImageData *image_data;
    TrainingConfig config;
} TrainingState;

// API functions
int start_training(TrainingConfig *config, TrainingState **state);
void stop_training(TrainingState *state);
void cleanup_training_state(TrainingState *state);
