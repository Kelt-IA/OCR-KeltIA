#include "../../include/nn/include_nn.h"
#include <math.h>
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
    for (size_t j = 0; j < layer->n_neurons; j++)
    {
        double sum = layer->bias[j];
        for (size_t i = 0; i < layer->n_inputs; i++)
        {
            sum += input[i] * layer->weights[j * layer->n_inputs + i];
        }

        layer->output[j] = f(sum);
    }
}

int create_layer(Layer *l, size_t n_inputs, size_t n_neurons, unsigned int SEED)
{
    l->n_inputs = n_inputs;
    l->n_neurons = n_neurons;

    l->bias = calloc(n_neurons, sizeof(double));
    l->output = calloc(n_neurons, sizeof(double));

    l->weights = calloc(n_inputs * n_neurons, sizeof(double));
    init_weights_deterministic(l->weights, n_inputs * n_neurons, SEED);

    if (!l->weights || !l->bias || !l->output)
    {
        fprintf(stderr, "layers.c: Error allocating memory \n");
        return 1;
    }

    return 0;
}

// This function is only used for the last layer of the
// neuronal network in order for the sum of all the outputs = 1
void softmax(Layer *layer)
{
    for (size_t j = 0; j < layer->n_neurons; j++)
    {
        double sum = 0;
        for (size_t i = 0; i < layer->n_inputs; i++)
        {
            sum += exp(layer->output[i]);
            // sum +=  input[i] * layer->weights[j * layer->n_inputs + i];
        }

        layer->output[j] = exp(layer->output[j]) / sum;
    }
}

void init_weights_deterministic(
    double *weights,
    size_t count,
    unsigned int seed
)
{
    // a fixed SEED for dev pourposes
    srand(seed);

    for (size_t i = 0; i < count; i++)
    {
        // Values between -0.5 y 0.5
        weights[i] = ((double)rand() / RAND_MAX) - 0.5;
    }
}

void load_weights(Layer *layer, double *weights)
{
    memcpy(
        layer->weights, weights,
        sizeof(double) * layer->n_neurons * layer->n_inputs
    );
}

void load_biases(Layer *layer, double *biases)
{
    memcpy(layer->bias, biases, sizeof(double) * layer->n_neurons);
}

ErrorCode load_weights_from_fs(FILE *f, Layer *layer)
{
    size_t count = layer->n_neurons * layer->n_inputs;
    size_t written = fread(layer->weights, sizeof(double), count, f);
    if (written != count)
    {
        if (feof(f))
        {
            fprintf(stderr, "layers.c: ERROR: End of file reached\n");
        }
        if (ferror(f))
        {
            fprintf(stderr, "layers.c: ERROR: File read error\n");
        }

        fprintf(
            stderr, "layers.c: (weights) written: %ld expected: %ld\n", written,
            count
        );
        return NN_ERR_READ;
    }

    return NN_ERR_OK;
}

ErrorCode load_biases_from_fs(FILE *f, Layer *layer)
{
    size_t count = layer->n_neurons;
    size_t written = fread(layer->bias, sizeof(double), count, f);
    if (written != count)
    {
        if (feof(f))
        {
            fprintf(stderr, "layers.c: ERROR: End of file reached\n");
        }
        if (ferror(f))
        {
            fprintf(stderr, "layers.c: ERROR: File read error\n");
        }
        fprintf(
            stderr, "layers.c: (biases) written: %ld expected: %ld\n", written,
            count
        );
        return NN_ERR_READ;
    }

    return NN_ERR_OK;
}
