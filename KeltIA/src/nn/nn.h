#pragma once

#include "layers.h"
#include <stdlib.h>

typedef struct
{
    size_t n_inputs;
    size_t n_layers;
    Layer *layers;

} NeuronalNetwork;

void free_nn(NeuronalNetwork *nn);
NeuronalNetwork *create_nn(size_t n_inputs, size_t n_layers,
                           size_t *neurons_per_layer);
void compute_nn(NeuronalNetwork *nn, double *input, double *output,
                Activation f);
