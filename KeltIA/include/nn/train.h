#pragma once

#include "accuracy_metrics.h"
#include "network.h"
#include <csignal>
#include <stddef.h>

// Global flag for SIGINT handling
extern volatile sig_atomic_t stop_requested;

// Signal handler
void global_sigint_handler(int sig);

// Training function
void train_nn(
    NeuronalNetwork *nn,
    Dataset *dataset,
    size_t epochs,
    size_t batch_size
);

// Dataset cleanup
void free_dataset(Dataset *dataset);
