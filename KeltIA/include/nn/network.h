#pragma once

#include "activation.h"
#include "layers.h"
#include <stdlib.h>

typedef struct
{
    size_t n_layers;
    Layer *layers;

} NeuronalNetwork;

void free_nn(NeuronalNetwork *nn);
void compute_nn(NeuronalNetwork *nn, double *input, double *output);

ErrorCode create_nn(
    size_t n_inputs,
    size_t n_layers,
    size_t neurons_per_layer[n_layers],
    NeuronalNetwork *out_nn
);
