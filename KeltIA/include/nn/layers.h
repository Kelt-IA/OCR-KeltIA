#pragma once

#include "activation.h"
#include <stdlib.h>

typedef struct
{
    size_t n_inputs;
    size_t n_neurons;

    double *weights;  // matriz [n_neurons * n_inputs]
    double *bias;     // array [n_neurons]
    double *output;   // array [n_neurons]
} Layer;

void free_layer(Layer *layer);
void foward_layer(Layer *layer, double *input, Activation f);
Layer create_layer(size_t n_inputs, size_t n_neurons);

void load_weights(Layer *layer, double *weights);
void load_biases(Layer *layer, double *weights);
