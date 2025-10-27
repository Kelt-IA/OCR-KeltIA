#pragma once

#include "layers.h"
#include "network.h"

double *delta_output(Layer *last_layer, double *expected);
double *delta_hidden_layer(Layer *layer, Layer *next_layer, double *next_delta);

double *gradient_weights(
    Layer *actual_layer,
    double *output_previous_layer,
    double *delta_actual_layer
);

double *gradient_biases(double *delta, size_t n_neurons);

void update_parameters(
    Layer *layer,
    double *grad_weights,
    double *grad_biases,
    double learning_rate
);

void backpropagation(
    NeuronalNetwork nn,
    double *input,
    double *expected_output
);
