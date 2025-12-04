// back-propagation.c
#include "../../include/nn/include_nn.h"
#include <cblas.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void delta_output(
    Layer *last_layer,
    double *expected,
    double *out_delta,
    size_t out_delta_size
)
{
    if (out_delta_size != last_layer->n_neurons)
    {
        fprintf(
            stderr,
            "back-propagation.c: Invalid length of array, actual: %ld, "
            "expected: %ld\n",
            out_delta_size, last_layer->n_neurons
        );
        exit(1);
    }

    if (!out_delta)
    {
        fprintf(stderr, "back-propagation.c: Invalid array pointer");
        exit(1);
    }

    for (size_t i = 0; i < last_layer->n_neurons; i++)
    {
        double error = last_layer->output[i] - expected[i];

        double deriv =
            last_layer->derivative_fn(last_layer->z[i], last_layer->output[i]);

        out_delta[i] = error * deriv;
    }
}

// Delta hidden layer: δ[l] = (W[l+1])^T * δ[l+1] * f'(z[l])
void delta_hidden_layer(
    Layer *layer,
    Layer *next_layer,
    double *next_delta,
    double *out_delta
)
{
    // 1. temp = (W[l+1])^T * δ[l+1]
    double *temp = (double *)malloc(layer->n_neurons * sizeof(double));
    if (!temp)
    {
        fprintf(stderr, "Error allocating temp for delta_hidden_layer\n");
        exit(1);
    }

    cblas_dgemv(
        CblasRowMajor, CblasTrans, next_layer->n_neurons, layer->n_neurons, 1.0,
        next_layer->weights, next_layer->n_inputs, next_delta, 1, 0.0, temp, 1
    );

    // 2. out_delta = temp * f'(z)
    for (size_t i = 0; i < layer->n_neurons; i++)
    {
        double deriv = layer->derivative_fn(layer->z[i], layer->output[i]);
        out_delta[i] = temp[i] * deriv;
    }

    free(temp);
}

// Gradient weights: ∂C/∂W = δ[l] * (a[l-1])^T   (outer product)
void gradient_weights(
    Layer *actual_layer,
    double *output_previous_layer,
    double *delta_actual_layer,
    double *out_gradient
)
{
    // outer product: grad_w += δ ⊗ a_prev^T
    cblas_dger(
        CblasRowMajor, actual_layer->n_neurons,
        actual_layer->n_inputs,    // M x N
        1.0,                       // alpha
        delta_actual_layer, 1,     // vector δ (M)
        output_previous_layer, 1,  // vector a_prev (N)
        out_gradient, actual_layer->n_inputs
    );
}

void gradient_biases(
    double *delta,
    size_t n_neurons,
    double *out_gradient_biases
)
{
    // grad_b += δ
    cblas_daxpy(n_neurons, 1.0, delta, 1, out_gradient_biases, 1);
}

void update_parameters(
    Layer *layer,
    double *grad_weights,
    double *grad_biases,
    double learning_rate
)
{
    size_t total_weights = layer->n_neurons * layer->n_inputs;

    // weights -= lr * grad_weights
    cblas_daxpy(
        total_weights, -learning_rate, grad_weights, 1, layer->weights, 1
    );

    // biases -= lr * grad_biases
    cblas_daxpy(
        layer->n_neurons, -learning_rate, grad_biases, 1, layer->bias, 1
    );
}

void get_empty_deltas(NeuronalNetwork *nn, double ***out_deltas)
{
    *out_deltas = (double **)malloc(nn->n_layers * sizeof(double *));

    if (!(*out_deltas))
    {
        fprintf(
            stderr, "back-propagation.c: Error allocating memory for deltas\n"
        );
        exit(1);
    }

    for (size_t i = 0; i < nn->n_layers; i++)
    {
        (*out_deltas)[i] =
            (double *)calloc(nn->layers[i].n_neurons, sizeof(double));

        if (!(*out_deltas)[i])
        {
            fprintf(
                stderr,
                "back-propagation.c: Error allocating memory for delta layer "
                "%zu\n",
                i
            );
            exit(1);
        }
    }
}

void get_empty_gradients(
    NeuronalNetwork *nn,
    double ***out_gradient_weights,
    double ***out_gradient_biases
)
{
    *out_gradient_biases = (double **)malloc(nn->n_layers * sizeof(double *));
    *out_gradient_weights = (double **)malloc(nn->n_layers * sizeof(double *));

    if (!(*out_gradient_biases) || !(*out_gradient_weights))
    {
        fprintf(
            stderr,
            "back-propagation.c: Error allocating memory for gradients\n"
        );
        exit(1);
    }

    for (size_t l = 0; l < nn->n_layers; l++)
    {
        Layer *actual_layer = &nn->layers[l];
        size_t size_grad_weights =
            actual_layer->n_neurons * actual_layer->n_inputs;

        (*out_gradient_biases)[l] =
            (double *)calloc(actual_layer->n_neurons, sizeof(double));
        (*out_gradient_weights)[l] =
            (double *)calloc(size_grad_weights, sizeof(double));

        if (!(*out_gradient_biases)[l] || !(*out_gradient_weights)[l])
        {
            fprintf(
                stderr,
                "back-propagation.c: Error allocating memory for layer %zu "
                "gradients\n",
                l
            );
            exit(1);
        }
    }
}

void backpropagation(
    NeuronalNetwork *nn,
    double *input,
    double *expected_output,
    double **deltas,
    double **grad_weights,
    double **grad_biases
)
{
    if (!deltas)
    {
        fprintf(stderr, "back-propagation.c: Invalid pointer provided");
        exit(1);
    }

    compute_nn(nn, input, NULL);

    // Backprop through dense layers
    if (nn->n_layers > 0)
    {
        Layer *last_layer = &nn->layers[nn->n_layers - 1];
        delta_output(
            last_layer, expected_output, deltas[nn->n_layers - 1],
            last_layer->n_neurons
        );

        for (int l = (int)nn->n_layers - 2; l >= 0; l--)
        {
            Layer *actual_layer = &nn->layers[l];
            Layer *next_layer = &nn->layers[l + 1];
            double *delta_next = deltas[l + 1];

            delta_hidden_layer(actual_layer, next_layer, delta_next, deltas[l]);
        }

        // Compute gradients for dense layers
        for (size_t l = 0; l < nn->n_layers; l++)
        {
            Layer *actual_layer = &nn->layers[l];
            double *previous_activation;

            if (l == 0)
            {
                // First dense layer: input comes from flattened conv or
                // original input
                previous_activation =
                    (nn->n_conv_layers > 0) ? nn->flattened : input;
            }
            else
            {
                previous_activation = nn->layers[l - 1].output;
            }

            gradient_weights(
                actual_layer, previous_activation, deltas[l], grad_weights[l]
            );
            gradient_biases(deltas[l], actual_layer->n_neurons, grad_biases[l]);
        }
    }

    // Backprop through conv layers with pooling support
    if (nn->n_conv_layers > 0)
    {
        double *grad_flattened = calloc(nn->flattened_size, sizeof(double));

        if (nn->n_layers > 0)
        {
            // Propagate gradient from first dense layer back to flattened
            Layer *first_dense = &nn->layers[0];
            cblas_dgemv(
                CblasRowMajor, CblasTrans, first_dense->n_neurons,
                first_dense->n_inputs, 1.0, first_dense->weights,
                first_dense->n_inputs, deltas[0], 1, 0.0, grad_flattened, 1
            );
        }

        double *grad_conv_output = grad_flattened;

        for (int i = (int)nn->n_conv_layers - 1; i >= 0; i--)
        {
            double *grad_kernels;
            double *grad_bias;
            get_empty_conv_gradients(
                &nn->conv_layers[i], &grad_kernels, &grad_bias
            );

            double *grad_pool_input = NULL;
            double *grad_input = NULL;

            // Backprop through pooling if exists
            if (nn->n_pool_layers > 0 && (size_t)i < nn->n_pool_layers)
            {
                // grad_conv_output is gradient w.r.t pool output
                // We need gradient w.r.t pool input (which is conv output)
                ConvLayer *conv = &nn->conv_layers[i];
                size_t pool_input_size =
                    conv->n_filters * conv->output_height * conv->output_width;
                grad_pool_input = calloc(pool_input_size, sizeof(double));

                backward_pool_layer(
                    &nn->pool_layers[i], grad_conv_output, grad_pool_input
                );

                // Now grad_pool_input is gradient w.r.t conv output
                // Backprop through conv layer
                if (i > 0)
                {
                    // Need gradient w.r.t previous pool output (or conv output
                    // if no pool)
                    if (nn->n_pool_layers > 0 &&
                        (size_t)(i - 1) < nn->n_pool_layers)
                    {
                        // Previous layer has pooling
                        PoolLayer *prev_pool = &nn->pool_layers[i - 1];
                        size_t prev_output_size = prev_pool->input_channels *
                                                  prev_pool->output_height *
                                                  prev_pool->output_width;
                        grad_input = calloc(prev_output_size, sizeof(double));
                    }
                    else
                    {
                        // Previous layer has no pooling
                        ConvLayer *prev_conv = &nn->conv_layers[i - 1];
                        size_t prev_output_size = prev_conv->n_filters *
                                                  prev_conv->output_height *
                                                  prev_conv->output_width;
                        grad_input = calloc(prev_output_size, sizeof(double));
                    }
                }

                backward_conv_layer(
                    &nn->conv_layers[i], grad_pool_input, grad_input,
                    grad_kernels, grad_bias
                );
                free(grad_pool_input);
            }
            else
            {
                // No pooling for this layer
                if (i > 0)
                {
                    ConvLayer *prev_conv = &nn->conv_layers[i - 1];
                    size_t prev_output_size = prev_conv->n_filters *
                                              prev_conv->output_height *
                                              prev_conv->output_width;
                    grad_input = calloc(prev_output_size, sizeof(double));
                }

                backward_conv_layer(
                    &nn->conv_layers[i], grad_conv_output, grad_input,
                    grad_kernels, grad_bias
                );
            }

            // Update conv layer parameters
            update_conv_parameters(
                &nn->conv_layers[i], grad_kernels, grad_bias, nn->learning_rate
            );

            free(grad_kernels);
            free(grad_bias);

            // Update grad_conv_output for next iteration
            if (i > 0)
            {
                if (grad_conv_output != grad_flattened)
                {
                    free(grad_conv_output);
                }
                grad_conv_output = grad_input;
            }
        }

        if (grad_conv_output != grad_flattened) { free(grad_conv_output); }
        free(grad_flattened);
    }
}
