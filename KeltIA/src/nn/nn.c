#include "layers.h"
#include <stdlib.h>

typedef struct
{
    size_t n_inputs;
    size_t n_layers;
    Layer *layers;

} NeuronalNetwork;

void free_nn(NeuronalNetwork *nn)
{
    if (!nn) return;

    size_t i = 0;
    while (i < nn->n_layers) { free_layer(&nn->layers[i]); }

    if (nn->layers) free(nn->layers);

    free(nn);
}

NeuronalNetwork *create_nn(size_t n_inputs, size_t n_layers,
                           size_t *neurons_per_layer)
{
    NeuronalNetwork *nn = malloc(sizeof(NeuronalNetwork));
    nn->n_inputs = n_inputs;
    nn->n_layers = n_layers;
    nn->layers = malloc(sizeof(Layer) * n_layers);

    for (size_t i = 0; i < n_layers; i++)
    {
        Layer *l = &nn->layers[i];
        l->n_inputs = (i == 0) ? n_inputs : neurons_per_layer[i - 1];
        l->n_neurons = neurons_per_layer[i];

        l->weights = malloc(sizeof(double) * l->n_inputs * l->n_neurons);
        l->bias = malloc(sizeof(double) * l->n_neurons);
        l->output = malloc(sizeof(double) * l->n_neurons);
    }

    return nn;
}
