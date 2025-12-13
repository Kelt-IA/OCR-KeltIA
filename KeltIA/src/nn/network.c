// network.c - Unified network with conv and dense layers
#include "../../include/nn/include_nn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_nn(NeuronalNetwork *nn)
{
    if (!nn) return;

    // Free conv layers
    if (nn->conv_layers)
    {
        for (size_t i = 0; i < nn->n_conv_layers; i++)
        {
            free_conv_layer(&nn->conv_layers[i]);
        }
        free(nn->conv_layers);
    }

    // Free dense layers
    if (nn->layers)
    {
        for (size_t i = 0; i < nn->n_layers; i++)
        {
            free_layer(&nn->layers[i]);
        }
        free(nn->layers);
    }

    // Free flattened buffer
    if (nn->flattened) free(nn->flattened);

    memset(nn, 0, sizeof(NeuronalNetwork));
}

ErrorCode create_nn(
    size_t n_inputs,
    size_t n_layers,
    size_t neurons_per_layer[n_layers],
    NeuronalNetwork *out_nn
)
{
    memset(out_nn, 0, sizeof(NeuronalNetwork));

    out_nn->n_conv_layers = 0;
    out_nn->n_layers = n_layers;
    out_nn->layers = malloc(sizeof(Layer) * n_layers);

    if (!out_nn->layers)
    {
        fprintf(stderr, "network.c: Error allocating layers\n");
        return NN_ERR_MEMORY;
    }

    for (size_t i = 0; i < n_layers; i++)
    {
        size_t inputs = (i == 0) ? n_inputs : neurons_per_layer[i - 1];

        int err = create_layer(
            &out_nn->layers[i], inputs, neurons_per_layer[i],
            i == n_layers - 1 ? ACTIVATION_SIGMOID : ACTIVATION_LEAKY_RELU
        );

        if (err != 0)
        {
            for (size_t j = 0; j < i; j++) { free_layer(&out_nn->layers[j]); }
            free(out_nn->layers);
            fprintf(stderr, "network.c: Error creating layer %zu\n", i);
            return NN_ERR_MEMORY;
        }
    }

    return NN_ERR_OK;
}

ErrorCode create_cnn(
    size_t n_conv_layers,
    ConvLayer *conv_configs,
    size_t n_dense_layers,
    size_t *dense_neurons,
    ActivationType *dense_activations,
    NeuronalNetwork *out_nn
)
{
    memset(out_nn, 0, sizeof(NeuronalNetwork));

    out_nn->n_conv_layers = n_conv_layers;
    out_nn->n_layers = n_dense_layers;

    // Allocate and copy conv layers
    if (n_conv_layers > 0)
    {
        out_nn->conv_layers = malloc(n_conv_layers * sizeof(ConvLayer));
        if (!out_nn->conv_layers)
        {
            fprintf(stderr, "network.c: Error allocating conv layers\n");
            return NN_ERR_MEMORY;
        }

        memcpy(
            out_nn->conv_layers, conv_configs, n_conv_layers * sizeof(ConvLayer)
        );

        ConvLayer *last_conv = &out_nn->conv_layers[n_conv_layers - 1];
        out_nn->flattened_size = last_conv->n_filters *
                                 last_conv->output_height *
                                 last_conv->output_width;

        out_nn->flattened = calloc(out_nn->flattened_size, sizeof(double));
        if (!out_nn->flattened)
        {
            fprintf(stderr, "network.c: Error allocating flattened buffer\n");
            free_nn(out_nn);
            return NN_ERR_MEMORY;
        }
    }

    // Allocate dense layers
    if (n_dense_layers > 0)
    {
        out_nn->layers = malloc(n_dense_layers * sizeof(Layer));
        if (!out_nn->layers)
        {
            fprintf(stderr, "network.c: Error allocating dense layers\n");
            free_nn(out_nn);
            return NN_ERR_MEMORY;
        }

        size_t first_input_size =
            (n_conv_layers > 0) ? out_nn->flattened_size : dense_neurons[0];

        for (size_t i = 0; i < n_dense_layers; i++)
        {
            size_t n_inputs =
                (i == 0) ? first_input_size : dense_neurons[i - 1];
            ActivationType activation =
                dense_activations
                    ? dense_activations[i]
                    : (i == n_dense_layers - 1 ? ACTIVATION_SIGMOID
                                               : ACTIVATION_LEAKY_RELU);

            int err = create_layer(
                &out_nn->layers[i], n_inputs, dense_neurons[i], activation
            );
            if (err != 0)
            {
                fprintf(
                    stderr, "network.c: Error creating dense layer %zu\n", i
                );
                free_nn(out_nn);
                return NN_ERR_MEMORY;
            }
        }
    }

    return NN_ERR_OK;
}

void compute_nn(NeuronalNetwork *nn, double *input, double *output)
{
    const double *current_input = input;

    // Forward through conv layers with optional pooling
    for (size_t i = 0; i < nn->n_conv_layers; i++)
    {
        forward_conv_layer(&nn->conv_layers[i], current_input);
        current_input = nn->conv_layers[i].output;
    }

    // Flatten conv/pool output if exists
    if (nn->n_conv_layers > 0)
    {
        ConvLayer *last_conv = &nn->conv_layers[nn->n_conv_layers - 1];
        memcpy(
            nn->flattened, last_conv->output,
            nn->flattened_size * sizeof(double)
        );
        current_input = nn->flattened;
    }

    // Forward through dense layers
    for (size_t l = 0; l < nn->n_layers; l++)
    {
        Layer *layer = &nn->layers[l];
        forward_layer(layer, (double *)current_input);
        current_input = layer->output;
    }

    // Copy last output
    if (nn->n_layers > 0 && output)
    {
        Layer *last = &nn->layers[nn->n_layers - 1];
        for (size_t i = 0; i < last->n_neurons; i++)
        {
            output[i] = last->output[i];
        }
    }
    else if (nn->n_conv_layers > 0 && output)
    {
        memcpy(output, nn->flattened, nn->flattened_size * sizeof(double));
    }
}
