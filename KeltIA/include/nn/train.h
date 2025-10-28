#include "network.h"

typedef struct
{
    double **inputs;
    double **targets;
    int num_samples;
    size_t input_size;
    size_t output_size;
} Dataset;

void average_gradients(
    NeuronalNetwork *nn,
    double **grad_weights,
    double **grad_biases,
    double batch_size
);

void reset_gradients(
    NeuronalNetwork *nn,
    double **grad_weights,
    double **grad_biases
);

void train_nn(
    NeuronalNetwork *nn,
    Dataset *dataset,
    size_t epochs,
    double batch
);
