#include "../../include/nn/include_nn.h"
#include <stdio.h>

int save_nn(char *path, const NeuronalNetwork *nn);

ErrorCode load_nn(const char *path, NeuronalNetwork *out_nn)
{
    if (!out_nn) return NN_ERR_NULL_POINTER;

    FILE *f = fopen(path, "rb");
    if (!f) return NN_ERR_FILE_OPEN;

    char magic[MAGIC_SIZE];
    fread(magic, sizeof(char), MAGIC_SIZE, f);

    // does the file have the MAGIC ?
    if (memcmp(magic, MAGIC, MAGIC_SIZE) != 0)
    {
        fclose(f);
        return NN_ERR_FORMAT;
    }

    fread(&out_nn->n_inputs, sizeof(int), 1, f);
    fread(&out_nn->n_layers, sizeof(int), 1, f);

    out_nn->layers = malloc(sizeof(Layer *) * out_nn->n_layers);
    if (!out_nn->layers)
    {
        fclose(f);
        return NN_ERR_MEMORY;
    }

    int prev_neurons = out_nn->n_inputs;
    for (int i = 0; i < out_nn->n_layers; i++)
    {
        fread(&out_nn->layers[i].n_neurons, sizeof(int), 1, f);
        // check error

        out_nn->layers[i].n_inputs = prev_neurons;

        // load weights
        ErrorCode err_weight = load_weights_from_fs(f, &out_nn->layers[i]);
        if (err_weight != 0)
        {
            printf("network_io: Error loading weights\n");
            return err_weight;
        }

        // load biases
        ErrorCode err_bias = load_weights_from_fs(f, &out_nn->layers[i]);
        if (err_bias != 0)
        {
            printf("network_io: Error loading biases\n");
            return err_bias;
        }

        prev_neurons = out_nn->layers[i].n_neurons;
    }

    return NN_ERR_OK;
}
