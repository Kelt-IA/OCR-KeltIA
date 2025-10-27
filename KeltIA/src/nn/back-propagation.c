#include "../../include/nn/include_nn.h"
#include <stdio.h>
#include <string.h>

double *delta_output(Layer *last_layer, double *expected)
{
    double *delta = (double *)malloc(last_layer->n_neurons * sizeof(double));
    if (!delta)
    {
        fprintf(
            stderr, "back-propagation.c: An error ocurred allocating memory"
        );
        exit(1);
    }

    for (size_t i = 0; i < last_layer->n_neurons; i++)
    {
        double error = last_layer->output[i] - expected[i];
        double deriv = last_layer->output[i] * (1 - last_layer->output[i]);

        delta[i] = error * deriv;
    }

    return delta;
}

double *delta_hidden_layer(Layer *layer, Layer *next_layer, double *next_delta)
{
    double *delta = (double *)malloc(layer->n_neurons * sizeof(double));
    if (!delta)
    {
        fprintf(
            stderr, "back-propagation.c: An error ocurred allocating memory"
        );
        exit(1);
    }

    for (size_t i = 0; i < layer->n_neurons; i++)
    {
        double sum = 0;
        for (size_t j = 0; j < next_layer->n_neurons; j++)
        {
            sum += WEIGHT(layer, i, j) * next_delta[i];
        }

        double deriv = layer->output[i] * (1 - layer->output[i]);

        delta[i] = sum * deriv;
    }

    return delta;
}

double *gradient_weights(
    Layer *actual_layer,
    double *output_previous_layer,
    double *delta_actual_layer
)
{
    double *grad_weights = (double *)malloc(
        actual_layer->n_neurons * actual_layer->n_inputs * sizeof(double)
    );

    if (!grad_weights)
    {
        fprintf(
            stderr, "back-propagation.c: An error ocurred allocating memory"
        );
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

            grad_weights[idx] =
                delta_actual_layer[i] * output_previous_layer[j];
        }
    }

    return grad_weights;
}

double *gradient_biases(double *delta, size_t n_neurons)
{
    // delta: delta of the `layer`
    // n_neurons: number of neurons that the `layer` has

    double *gradient_biases = (double *)malloc(n_neurons * sizeof(double));
    if (!gradient_biases)
    {
        fprintf(
            stderr, "back-propagation.c: An error ocurred allocating memory"
        );
        exit(1);
    }

    // the gradient of the biases is delta
    // ∂C/∂b[l] = δ[l]
    memccpy(gradient_biases, delta, n_neurons, sizeof(double));

    return gradient_biases;
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

void backpropagation(NeuronalNetwork nn, double *input, double *expected_output)
{
    double **deltas = (double **)malloc(nn.n_layers * sizeof(double *));
    if (!deltas)
    {
        fprintf(
            stderr, "back-propagation.c: An error ocurred allocating memory"
        );
        exit(1);
    }

    deltas[nn.n_layers - 1] =
        delta_output(&nn.layers[nn.n_layers - 1], expected_output);

    for (size_t l = nn.n_layers - 2; l != 0; l--)
    {
        Layer *actual_layer = &nn.layers[l];
        Layer *next_layer = &nn.layers[l + 1];
        double *delta_next = deltas[l + 1];

        deltas[l] = delta_hidden_layer(actual_layer, next_layer, delta_next);
    }

    for (size_t l = 0; l < nn.n_layers; l++)
    {
        Layer *actual_layer = &nn.layers[l];
        double *previous_activation;

        if (l == 0)
            previous_activation = input;
        else
            previous_activation = nn.layers[l - 1].output;

        double *grad_weights =
            gradient_weights(actual_layer, previous_activation, deltas[l]);
        double *grad_biases =
            gradient_biases(deltas[l], actual_layer->n_neurons);

        update_parameters(
            actual_layer, grad_weights, grad_biases, LEARNING_RATE
        );

        free(grad_weights);
        free(grad_biases);
    }

    // frees
    for (size_t i = 0; i < nn.n_layers; i++) { free(deltas[i]); }
    free(deltas);
}
