#pragma once

#include "network.h"
#include <stddef.h>

typedef struct
{
    double accuracy;
    double mse;
    int correct_predictions;
} EvaluationMetrics;

typedef struct
{
    double **inputs;   //  [ [1.0, 0.0, ...], [], []     ]
    double **targets;  //  [ [1], [9], [8], ... ]
    int num_samples;
    size_t input_size;
    size_t output_size;
} Dataset;

// Evaluate network performance
EvaluationMetrics evaluate_network(NeuronalNetwork *nn, Dataset *dataset);

// Log metrics to file
void log_metrics(const char *filepath, size_t epoch, EvaluationMetrics metrics);
