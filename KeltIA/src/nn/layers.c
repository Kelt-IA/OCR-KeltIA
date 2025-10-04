#include "../../include/nn/include_nn.h"
#include <stdio.h>
#include <stdlib.h>
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

int create_layer(Layer *l, size_t n_inputs, size_t n_neurons)
{
    l->n_inputs = n_inputs;
    l->n_neurons = n_neurons;

    l->weights = calloc(n_inputs * n_neurons, sizeof(double));
    l->bias = calloc(n_neurons, sizeof(double));
    l->output = calloc(n_neurons, sizeof(double));

    if (!l->weights || !l->bias || !l->output)
    {
        fprintf(stderr, "layers.c: Error allocating memory \n");
        return 1;
    }

    return 0;
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

ErrorCode load_weights_from_fs(FILE *f, Layer *layer)
{
    size_t count = layer->n_neurons * layer->n_inputs;
    if (fread(layer->weights, sizeof(double), count, f) != count)
    {
        return NN_ERR_READ;
    }

    return NN_ERR_OK;
}

ErrorCode load_biases_from_fs(FILE *f, Layer *layer)
{
    size_t count = layer->n_neurons;
    if (fread(layer->bias, sizeof(double), count, f) != count)
    {
        return NN_ERR_READ;
    }

    return NN_ERR_OK;
}
