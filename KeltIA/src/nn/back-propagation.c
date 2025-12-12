// back-propagation.c
#include "../../include/nn/include_nn.h"
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
    // Compute (W[l+1])^T * δ[l+1]
    // W_next is [n_neurons_next x n_inputs_next]
    // We want: W_next^T * delta_next

    for (size_t i = 0; i < layer->n_neurons; i++)
    {
        double sum = 0.0;

        // Sum over all neurons in next layer
        for (size_t j = 0; j < next_layer->n_neurons; j++)
        {
            // W_next[j][i] * delta_next[j]
            sum += next_layer->weights[j * next_layer->n_inputs + i] *
                   next_delta[j];
        }

        // Multiply by activation derivative
        double deriv = layer->derivative_fn(layer->z[i], layer->output[i]);
        out_delta[i] = sum * deriv;
    }
}

// Gradient weights: ∂C/∂W = δ[l] * (a[l-1])^T (outer product)
void gradient_weights(
    Layer *actual_layer,
    double *output_previous_layer,
    double *delta_actual_layer,
    double *out_gradient
)
{
    // Outer product: grad_w += δ ⊗ a_prev^T
    // delta: [n_neurons x 1]
    // previous: [n_inputs x 1]
    // result: [n_neurons x n_inputs]

    for (size_t i = 0; i < actual_layer->n_neurons; i++)
    {
        for (size_t j = 0; j < actual_layer->n_inputs; j++)
        {
            out_gradient[i * actual_layer->n_inputs + j] +=
                delta_actual_layer[i] * output_previous_layer[j];
        }
    }
}

void gradient_biases(
    double *delta,
    size_t n_neurons,
    double *out_gradient_biases
)
{
    // grad_b += δ
    for (size_t i = 0; i < n_neurons; i++)
    {
        out_gradient_biases[i] += delta[i];
    }
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
    for (size_t i = 0; i < total_weights; i++)
    {
        layer->weights[i] -= learning_rate * grad_weights[i];
    }

    // biases -= lr * grad_biases
    for (size_t i = 0; i < layer->n_neurons; i++)
    {
        layer->bias[i] -= learning_rate * grad_biases[i];
    }
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
    double **grad_biases,
    double **conv_grad_kernels,
    double **conv_grad_bias
)
{
    if (!deltas)
    {
        fprintf(stderr, "back-propagation.c: Invalid pointer provided\n");
        exit(1);
    }

    // Forward pass
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

    // Backprop through conv layers
    if (nn->n_conv_layers > 0 && conv_grad_kernels && conv_grad_bias)
    {
        double *grad_flattened = calloc(nn->flattened_size, sizeof(double));
        if (!grad_flattened)
        {
            fprintf(stderr, "Error allocating grad_flattened\n");
            exit(1);
        }

        if (nn->n_layers > 0)
        {
            // Propagate gradient from first dense layer back to flattened
            // grad_flattened = W^T * delta[0]
            Layer *first_dense = &nn->layers[0];

            for (size_t i = 0; i < first_dense->n_inputs;
                 i++)  // flattened_size
            {
                double sum = 0.0;

                for (size_t j = 0; j < first_dense->n_neurons; j++)
                {
                    sum += first_dense->weights[j * first_dense->n_inputs + i] *
                           deltas[0][j];
                }

                grad_flattened[i] = sum;
            }
        }

        // Backprop through each conv layer
        double *current_grad = grad_flattened;

        for (int i = (int)nn->n_conv_layers - 1; i >= 0; i--)
        {
            double *next_grad = NULL;

            // Allocate gradient for previous layer (if exists)
            if (i > 0)
            {
                ConvLayer *prev_conv = &nn->conv_layers[i - 1];
                size_t prev_output_size = prev_conv->n_filters *
                                          prev_conv->output_height *
                                          prev_conv->output_width;
                next_grad = calloc(prev_output_size, sizeof(double));
                if (!next_grad)
                {
                    fprintf(stderr, "Error allocating next_grad\n");
                    // Cleanup
                    if (current_grad != grad_flattened) free(current_grad);
                    free(grad_flattened);
                    exit(1);
                }
            }

            // Compute gradients for this conv layer
            backward_conv_layer(
                &nn->conv_layers[i], current_grad,
                next_grad,  // Can be NULL for first conv layer
                conv_grad_kernels[i], conv_grad_bias[i]
            );

            // Free current gradient if it's not the original grad_flattened
            if (current_grad != grad_flattened) { free(current_grad); }

            // Move to next iteration
            current_grad = next_grad;
        }

        // Free remaining gradients
        if (current_grad != NULL && current_grad != grad_flattened)
        {
            free(current_grad);
        }

        free(grad_flattened);
    }
}
