#pragma once

#include "layers.h"
#include "network.h"

void delta_output(
    Layer *last_layer,
    double *expected,
    double *out_delta,
    size_t out_delta_size
);

void delta_hidden_layer(
    Layer *layer,
    Layer *next_layer,
    double *next_delta,
    double *out_delta
);

// void gradient_weights(
//     Layer *actual_layer,
//     double *output_previous_layer,
//     double *delta_actual_layer,
//     double *out_gradient
// );
//
// void gradient_biases(
//     double *delta,
//     size_t n_neurons,
//     double *out_gradient_biases
// );

void update_parameters(
    Layer *layer,
    double *grad_weights,
    double *grad_biases,
    double learning_rate
);

void backpropagation(
    NeuronalNetwork *nn,
    double *input,
    double *expected_output,
    double **deltas,
    double **grad_weights,
    double **grad_biases,
    double **conv_grad_kernels,  // NUEVO
    double **conv_grad_bias      // NUEVO
);

void get_empty_deltas(NeuronalNetwork *nn, double ***out_deltas);

void get_empty_gradients(
    NeuronalNetwork *nn,
    double ***out_gradient_weights,
    double ***out_gradient_biases
);
