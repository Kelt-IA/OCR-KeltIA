#include "../../include/nn/include_nn.h"
#include <stdio.h>

void print_double_array(const double *arr, size_t size)
{
    printf("[");
    for (size_t i = 0; i < size; i++)
    {
        printf("%.6f", arr[i]);
        if (i < size - 1) { printf(", "); }
    }
    printf("]\n");
}

ErrorCode save_nn(const char *path, const NeuronalNetwork *nn)
{
    if (!nn)
    {
        fprintf(stderr, "network_io: NeuronalNetwork pointer is null");
        return NN_ERR_NULL_POINTER;
    }

    if (!path)
    {
        fprintf(stderr, "network_io: path pointer is null");
        return NN_ERR_NULL_POINTER;
    }

    FILE *f = fopen(path, "w");
    if (!f) return NN_ERR_FILE_OPEN;

    // write header
    fwrite(MAGIC, MAGIC_SIZE, 1, f);
    // fwrite(&nn->n_inputs, sizeof(size_t), 1, f);
    fwrite(&nn->n_layers, sizeof(size_t), 1, f);

    // layers
    size_t count_written;
    for (size_t i = 0; i < nn->n_layers; i++)
    {
        // structure
        // {n_neurons_layer_i}
        // {activation_type}
        // {biases_layer_i}
        // {weights_layer_i}

        fwrite(&nn->layers[i].n_neurons, sizeof(size_t), 1, f);
        fwrite(&nn->layers[i].activation_type, sizeof(int), 1, f);

        count_written = fwrite(
            nn->layers[i].bias, sizeof(double), nn->layers[i].n_neurons, f
        );

        if (count_written != nn->layers[i].n_neurons)
        {
            fclose(f);
            fprintf(stderr, "network_io.c: Error writting biases to file\n");
            return NN_ERR_WRITE;
        }

        count_written = fwrite(
            nn->layers[i].weights, sizeof(double),
            nn->layers[i].n_neurons * nn->layers[i].n_inputs, f
        );

        if (count_written != nn->layers[i].n_neurons * nn->layers[i].n_inputs)
        {
            fclose(f);
            fprintf(stderr, "network_io.c: Error writing weights to file\n");
            return NN_ERR_WRITE;
        }
    }

    fclose(f);
    return NN_ERR_OK;
}

ErrorCode load_nn(const char *path, NeuronalNetwork *out_nn)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NN_ERR_FILE_OPEN;

    char magic[MAGIC_SIZE];
    fread(magic, sizeof(char), MAGIC_SIZE, f);

    // does the file have the MAGIC ?
    if (memcmp(magic, MAGIC, MAGIC_SIZE) != 0)
    {
        fclose(f);
        fprintf(
            stderr,
            "network_io: file provided does not have the correct format\n"
        );
        return NN_ERR_FORMAT;
    }

    // fread(&out_nn->n_inputs, sizeof(size_t), 1, f);
    fread(&out_nn->n_layers, sizeof(size_t), 1, f);

    out_nn->layers = malloc(sizeof(Layer) * out_nn->n_layers);
    if (!out_nn->layers)
    {
        fclose(f);
        fprintf(stderr, "network_io: Error alocating memory\n");
        return NN_ERR_MEMORY;
    }

    for (size_t i = 0; i < out_nn->n_layers; i++)
    {
        // reading the layers

        size_t neurons;
        fread(&neurons, sizeof(size_t), 1, f);

        int prev_neurons;
        if (i == 0) { prev_neurons = neurons; }
        else
        {
            prev_neurons = out_nn->layers[i - 1].n_neurons;
        }

        // read activation
        int type;
        fread(&type, sizeof(int), 1, f);

        int err = create_layer(
            &out_nn->layers[i], prev_neurons, neurons, ACTIVATION_LEAKY_RELU
        );
        if (err != 0)
        {
            fclose(f);
            fprintf(stderr, "network_io.c: Error creating layer\n");
            return NN_ERR_NULL_POINTER;
        }

        set_activation(&out_nn->layers[i], int_to_activation(type));

        // load weights
        ErrorCode err_weight = load_biases_from_fs(f, &out_nn->layers[i]);
        if (err_weight != 0)
        {
            fprintf(stderr, "network_io: Error loading biases\n");
            return err_weight;
        }

        // load biases
        ErrorCode err_bias = load_weights_from_fs(f, &out_nn->layers[i]);
        if (err_bias != 0)
        {
            fprintf(stderr, "network_io: Error loading weights\n");
            return err_bias;
        }

        prev_neurons = out_nn->layers[i].n_neurons;
    }

    fclose(f);
    return NN_ERR_OK;
}
