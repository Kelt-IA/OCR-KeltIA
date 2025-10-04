#include "../../include/nn/include_nn.h"
#include <stdio.h>

void free_nn(NeuronalNetwork *nn)
{
    if (!nn) return;

    for (size_t i = 0; i < nn->n_layers; i++) { free_layer(&nn->layers[i]); }

    if (nn->layers) free(nn->layers);

    free(nn);
}

NeuronalNetwork *create_nn(size_t n_inputs, size_t n_layers,
                           size_t neurons_per_layer[n_layers])
{
    NeuronalNetwork *nn = malloc(sizeof(NeuronalNetwork));
    nn->n_inputs = n_inputs;
    nn->n_layers = n_layers;
    nn->layers = malloc(sizeof(Layer) * n_layers);

    for (size_t i = 0; i < n_layers; i++)
    {
        Layer *l = calloc(1, sizeof(Layer));

        int inputs = (i == 0) ? n_inputs : neurons_per_layer[i - 1];
        int err = create_layer(l, inputs, neurons_per_layer[i]);
        if (err != 0)
        {
            fprintf(stderr, "network.c: Error creating layer\n");
            return NULL;
        }

        /*
        Layer *l = &nn->layers[i];
        l->n_inputs = (i == 0) ? n_inputs : neurons_per_layer[i - 1];
        l->n_neurons = neurons_per_layer[i];

        l->weights = malloc(sizeof(double) * l->n_inputs * l->n_neurons);
        l->bias = malloc(sizeof(double) * l->n_neurons);
        l->output = malloc(sizeof(double) * l->n_neurons);
         */
    }

    return nn;
}

void compute_nn(NeuronalNetwork *nn, double *input, double *output,
                Activation f)
{
    double *in = input;

    for (size_t l = 0; l < nn->n_layers; l++)
    {
        Layer *layer = &nn->layers[l];

        foward_layer(layer, in, f);

        // Para la siguiente capa, la entrada es la salida actual
        in = layer->output;
    }

    // Copiar salida final
    Layer *last = &nn->layers[nn->n_layers - 1];
    for (size_t i = 0; i < last->n_neurons; i++) output[i] = last->output[i];
}
