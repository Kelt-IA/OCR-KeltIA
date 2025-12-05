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
    if (layer->z) free(layer->z);
    if (layer->output) free(layer->output);
}

void set_activation(Layer *layer, ActivationType type)
{
    layer->activation_type = type;

    switch (type)
    {

    case ACTIVATION_SIGMOID:
        layer->activation_fn = sigmoid;
        layer->derivative_fn = sigmoid_derivative;
        break;

    case ACTIVATION_LEAKY_RELU:
        layer->activation_fn = leaky_relu;
        layer->derivative_fn = leaky_relu_derivative;
        break;

    case ACTIVATION_STEP:
        layer->activation_fn = step;
        layer->derivative_fn = NULL;  // not used
        break;
    }
}

void foward_layer(Layer *layer, double *input)
{
    // z = W * input + bias
    // cblas_dgemv(
    //     CblasRowMajor, CblasNoTrans, layer->n_neurons,
    //     layer->n_inputs,                  // M x N de W
    //     1.0,                              // alpha
    //     layer->weights, layer->n_inputs,  // W, lda = n_inputs
    //     input, 1,                         // vector input, incx=1
    //     0.0, layer->z, 1
    // );
    //
    // // bias: z += bias
    // cblas_daxpy(layer->n_neurons, 1.0, layer->bias, 1, layer->z, 1);

    for (size_t i = 0; i < layer->n_neurons; i++)
    {
        double sum = layer->bias[i];  // Start with bias

        // Dot product: row i of W with input
        for (size_t j = 0; j < layer->n_inputs; j++)
        {
            sum += layer->weights[i * layer->n_inputs + j] * input[j];
        }

        layer->z[i] = sum;
    }

    for (size_t j = 0; j < layer->n_neurons; j++)
    {
        layer->output[j] = layer->activation_fn(layer->z[j]);
    }
}

int create_layer(
    Layer *l,
    size_t n_inputs,
    size_t n_neurons,
    ActivationType activation
)
{
    memset(l, 0, sizeof(Layer));

    l->n_inputs = n_inputs;
    l->n_neurons = n_neurons;

    l->bias = calloc(n_neurons, sizeof(double));
    l->weights = calloc(n_inputs * n_neurons, sizeof(double));
    init_weights(l);

    l->z = calloc(n_neurons, sizeof(double));
    l->output = calloc(n_neurons, sizeof(double));

    if (!l->weights || !l->bias || !l->output || !l->z)
    {
        fprintf(stderr, "layers.c: Error allocating memory \n");
        return 1;
    }

    set_activation(l, activation);

    return 0;
}

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

void init_weights(Layer *layer)
{
    double xavier_limit = sqrt(6.0 / (layer->n_inputs + layer->n_neurons));

    for (size_t i = 0; i < (layer->n_neurons * layer->n_inputs); i++)
    {
        // Distribución uniforme en [-xavier_limit, +xavier_limit]
        layer->weights[i] =
            ((double)rand() / RAND_MAX) * 2.0 * xavier_limit - xavier_limit;
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
