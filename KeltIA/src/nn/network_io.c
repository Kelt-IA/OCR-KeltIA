#include "../../include/nn/include_nn.h"
#include <stdio.h>

int save_nn(char *path, const NeuronalNetwork *nn);

ErrorCode load_nn(const char *path, NeuronalNetwork *out_nn)
{
    FILE *f = fopen(path, "rb");
    if (!f) return ERR_FILE_OPEN;

    char magic[MAGIC_SIZE];
    fread(magic, sizeof(char), MAGIC_SIZE, f);

    // does the file have the MAGIC ?
    if (memcmp(magic, MAGIC, MAGIC_SIZE) != 0)
    {
        fclose(f);
        return ERR_FORMAT;
    }

    fread(&out_nn->n_inputs, sizeof(int), 1, f);
    fread(&out_nn->n_layers, sizeof(int), 1, f);

    out_nn->layers = malloc(sizeof(Layer *) * out_nn->n_layers);
    if (!out_nn->layers)
    {
        fclose(f);
        return ERR_MEMORY;
    }

    return ERR_OK;
}
