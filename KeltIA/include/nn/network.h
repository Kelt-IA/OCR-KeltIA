#pragma once

#include "activation.h"
#include "convolution.h"
#include "layers.h"
#include <stdlib.h>

typedef struct
{
    size_t n_conv_layers;
    ConvLayer *conv_layers;

    // Flattened output from conv layers (input to dense layers)
    double *flattened;
    size_t flattened_size;

    size_t n_layers;
    Layer *layers;

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
    size_t n_dense_layers,
    size_t *dense_neurons,
    ActivationType *dense_activations,
    NeuronalNetwork *out_nn
);

void free_nn(NeuronalNetwork *nn);
void compute_nn(NeuronalNetwork *nn, double *input, double *output);
