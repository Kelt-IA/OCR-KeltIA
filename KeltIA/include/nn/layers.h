#pragma once

#include "activation.h"
#include "errors.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    size_t n_inputs;
    size_t n_neurons;

    double *weights;  // matrix [n_neurons * n_inputs]
    double *bias;     // array [n_neurons]
    double *z;        // array [n_neurons]
    double *output;   // array [n_neurons]

    ActivationType activation_type;
    ActivationFunction activation_fn;
    DerivativeFunction derivative_fn;
} Layer;

#define WEIGHT(layer, i, j) ((layer)->weights[(i) * (layer)->n_inputs + (j)])

void free_layer(Layer *layer);
void forward_layer(Layer *layer, double *input);
void softmax(Layer *layer);

int create_layer(
    Layer *layer,
    size_t n_inputs,
    size_t n_neurons,
    ActivationType activation
);

void load_weights(Layer *layer, double *weights);
void load_biases(Layer *layer, double *weights);

void init_weights(Layer *layer);
void set_activation(Layer *layer, ActivationType type);

ErrorCode load_weights_from_fs(FILE *f, Layer *layer);
ErrorCode load_biases_from_fs(FILE *f, Layer *layer);
