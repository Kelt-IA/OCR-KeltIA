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

    // double *delta = (double *)malloc(last_layer->n_neurons * sizeof(double));
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

void delta_hidden_layer(
    Layer *layer,
    Layer *next_layer,
    double *next_delta,
    double *out_delta,
    size_t out_delta_size
)
{
    if (out_delta_size != layer->n_neurons)
    {
        fprintf(
            stderr,
            "back-propagation.c: Invalid length of array, actual: %ld, "
            "expected: %ld\n",
            out_delta_size, layer->n_neurons
        );
        exit(1);
    }

    // double *delta = (double *)malloc(layer->n_neurons * sizeof(double));
    if (!out_delta)
    {
        fprintf(stderr, "back-propagation.c: Invalid array pointer");
        exit(1);
    }

    for (size_t i = 0; i < layer->n_neurons; i++)
    {
        double sum = 0;
        for (size_t j = 0; j < next_layer->n_neurons; j++)
        {
            sum += WEIGHT(next_layer, j, i) * next_delta[j];
        }

        double deriv = layer->derivative_fn(layer->z[i], layer->output[i]);
        // double deriv = layer->output[i] * (1 - layer->output[i]);

        out_delta[i] = sum * deriv;
    }
}

void gradient_weights(
    Layer *actual_layer,
    double *output_previous_layer,
    double *delta_actual_layer,
    double *out_gradient,
    size_t out_gradient_size
)
{
    if (out_gradient_size != actual_layer->n_neurons * actual_layer->n_inputs)
    {
        fprintf(
            stderr,
            "back-propagation.c: Invalid size of array, actual: %ld, expected: "
            "%ld",
            out_gradient_size, actual_layer->n_neurons * actual_layer->n_inputs
        );
        exit(1);
    }
    // out_gradient_size =

    // double *grad_weights = (double *)malloc(
    //     actual_layer->n_neurons * actual_layer->n_inputs * sizeof(double)
    // );

    if (!out_gradient)
    {
        fprintf(stderr, "back-propagation.c: Invalid pointer provided");
        exit(1);
    }

    // exterior product: δ[l] * (a[l-1])^T
    // the gradients are calculated multipliying the delta of each neuron
    // with the activations of the previous layer

    for (size_t i = 0; i < actual_layer->n_neurons; i++)
    {
        for (size_t j = 0; j < actual_layer->n_inputs; j++)
        {
            // from 2D to 1D
            int idx = i * actual_layer->n_inputs + j;

            // The gradient of the weight that conects neuron j of
            // previous_layer with neuron i of actual_layer:
            // ∂C/∂w[i][j] = δ[i] * preious_a[j]

            out_gradient[idx] =
                delta_actual_layer[i] * output_previous_layer[j];
        }
    }
}

void gradient_biases(
    double *delta,
    size_t n_neurons,
    double *out_gradient_biases,
    size_t out_gradient_size
)
{
    if (out_gradient_size != n_neurons)
    {
        fprintf(
            stderr,
            "back-propagation.c: Invalid length of array, actual: %ld, "
            "expected: %ld\n",
            out_gradient_size, n_neurons
        );
        exit(1);
    }

    // delta: delta of the `layer`
    // n_neurons: number of neurons that the `layer` has

    // double *gradient_biases = (double *)malloc(n_neurons * sizeof(double));
    if (!out_gradient_biases)
    {
        fprintf(stderr, "back-propagation.c: Invalid pointer provided");
        exit(1);
    }

    // the gradient of the biases is delta
    // ∂C/∂b[l] = δ[l]
    memcpy(out_gradient_biases, delta, n_neurons * sizeof(double));
}

void update_parameters(
    Layer *layer,
    double *grad_weights,
    double *grad_biases,
    double learning_rate
)
{
    size_t total_weights = layer->n_neurons * layer->n_inputs;
    for (size_t idx = 0; idx < total_weights; idx++)
    {
        layer->weights[idx] -= learning_rate * grad_weights[idx];
    }

    for (size_t idx = 0; idx < layer->n_neurons; idx++)
    {
        layer->bias[idx] -= learning_rate * grad_biases[idx];
    }

    return;
}

void get_empty_deltas(NeuronalNetwork nn, double ***out_deltas)
{
    *out_deltas = (double **)malloc(nn.n_layers * sizeof(double *));

    if (!(*out_deltas))
    {
        fprintf(
            stderr, "back-propagation.c: Error allocating memory for deltas\n"
        );
        exit(1);
    }

    for (size_t i = 0; i < nn.n_layers; i++)
    {
        (*out_deltas)[i] =
            (double *)calloc(nn.layers[i].n_neurons, sizeof(double));

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
    NeuronalNetwork nn,
    double ***out_gradient_weights,
    double ***out_gradient_biases
)
{
    *out_gradient_biases = (double **)malloc(nn.n_layers * sizeof(double *));
    *out_gradient_weights = (double **)malloc(nn.n_layers * sizeof(double *));

    if (!(*out_gradient_biases) || !(*out_gradient_weights))
    {
        fprintf(
            stderr,
            "back-propagation.c: Error allocating memory for gradients\n"
        );
        exit(1);
    }

    for (size_t l = 0; l < nn.n_layers; l++)
    {
        Layer *actual_layer = &nn.layers[l];
        size_t size_grad_weights =
            actual_layer->n_neurons * actual_layer->n_inputs;

        (*out_gradient_biases)[l] =
            (double *)malloc(actual_layer->n_neurons * sizeof(double));
        (*out_gradient_weights)[l] =
            (double *)malloc(size_grad_weights * sizeof(double));

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

        delta_hidden_layer(
            actual_layer, next_layer, delta_next, deltas[l],
            actual_layer->n_neurons
        );
    }

    for (size_t l = 0; l < nn->n_layers; l++)
    {
        Layer *actual_layer = &nn->layers[l];
        double *previous_activation;

        if (l == 0)
            previous_activation = input;
        else
            previous_activation = nn->layers[l - 1].output;

        size_t size_grad_weights =
            actual_layer->n_neurons * actual_layer->n_inputs;

        gradient_weights(
            actual_layer, previous_activation, deltas[l], grad_weights[l],
            size_grad_weights
        );

        gradient_biases(
            deltas[l], actual_layer->n_neurons, grad_biases[l],
            actual_layer->n_neurons
        );
    }
}
