#include "../../include/nn/include_nn.h"
#include <stdio.h>

void free_nn(NeuronalNetwork *nn)
{
    if (!nn) return;

    for (size_t i = 0; i < nn->n_layers; i++) { free_layer(&nn->layers[i]); }

    if (nn->layers) free(nn->layers);
}

ErrorCode create_nn(
    size_t n_inputs,
    size_t n_layers,
    size_t neurons_per_layer[n_layers],
    NeuronalNetwork *out_nn,
    unsigned int SEED
)
{
    out_nn->n_inputs = n_inputs;
    out_nn->n_layers = n_layers;
    out_nn->layers = malloc(sizeof(Layer) * n_layers);

    for (size_t i = 0; i < n_layers; i++)
    {
        int inputs = (i == 0) ? n_inputs : neurons_per_layer[i - 1];
        int err = create_layer(
            &out_nn->layers[i], inputs, neurons_per_layer[i], SEED
        );
        if (err != 0)
        {
            for (size_t j = 0; j < i; j++) { free_layer(&out_nn->layers[j]); }
            free(out_nn->layers);
            fprintf(stderr, "network.c: Error creating layer\n");
            return NN_ERR_MEMORY;
        }
    }

    return NN_ERR_OK;
}

void compute_nn(
    NeuronalNetwork *nn,
    double *input,
    double *output,
    Activation f
)
{
    double *in = input;
    for (size_t l = 0; l < nn->n_layers; l++)
    {
        Layer *layer = &nn->layers[l];
        foward_layer(layer, in, f);

        // foward the output of the layer to the input of the next
        in = layer->output;
    }

    // Copy last output
    Layer *last = &nn->layers[nn->n_layers - 1];
    for (size_t i = 0; i < last->n_neurons; i++) output[i] = last->output[i];

    // ensure sum of all outputs = 1
    // softmax(last);
}
