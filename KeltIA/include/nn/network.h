#pragma once

#include "activation.h"
#include "convolution.h"
#include "layers.h"
#include "pooling.h"
#include <stdlib.h>

typedef struct
{
    // Convolutional part
    size_t n_conv_layers;
    ConvLayer *conv_layers;

    // Pooling layers (one after each conv layer)
    size_t n_pool_layers;
    PoolLayer *pool_layers;

    // Flattening between conv and dense
    double *flattened;
    size_t flattened_size;

    // Dense part
    size_t n_layers;
    Layer *layers;

    // Learning rate
    double learning_rate;
} NeuronalNetwork;

ErrorCode create_nn(
    size_t n_inputs,
    size_t n_layers,
    size_t neurons_per_layer[n_layers],
    NeuronalNetwork *out_nn
);

ErrorCode create_cnn(
    size_t n_conv_layers,
    ConvLayer *conv_configs,
    int use_pooling,
    size_t n_dense_layers,
    size_t *dense_neurons,
    ActivationType *dense_activations,
    NeuronalNetwork *out_nn
);

void free_nn(NeuronalNetwork *nn);
void compute_nn(NeuronalNetwork *nn, double *input, double *output);
