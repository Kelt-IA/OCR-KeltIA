#include "../../include/nn/include_nn.h"
#include <string.h>

void free_layer(Layer *layer)
{
    if (!layer) return;
    if (layer->weights) free(layer->weights);
    if (layer->bias) free(layer->bias);
    if (layer->output) free(layer->output);
}

void foward_layer(Layer *layer, double *input, Activation f)
{
    for (int j = 0; j < layer->n_neurons; j++)
    {
        double sum = layer->bias[j];
        for (int i = 0; i < layer->n_inputs; i++)
        {
            sum += input[i] * layer->weights[j * layer->n_inputs + i];
        }
        layer->output[j] = f(sum);
    }
}

Layer create_layer(size_t n_inputs, size_t n_neurons)
{
    Layer l;
    l.n_inputs = n_inputs;
    l.n_neurons = n_neurons;

    l.weights = malloc(sizeof(double) * n_inputs * n_neurons);
    l.bias = malloc(sizeof(double) * n_neurons);
    l.output = malloc(sizeof(double) * n_neurons);

    // Initialize weights and biases
    for (size_t i = 0; i < n_inputs * n_neurons; i++) l.weights[i] = 0.0;
    for (size_t i = 0; i < n_neurons; i++) l.bias[i] = 0.0;
    for (size_t i = 0; i < n_neurons; i++) l.output[i] = 0.0;

    return l;
}

void load_weights(Layer *layer, double *weights)
{
    memcpy(layer->weights, weights,
           sizeof(double) * layer->n_neurons * layer->n_inputs);
}

void load_biases(Layer *layer, double *biases)
{
    memcpy(layer->bias, biases, sizeof(double) * layer->n_neurons);
}
