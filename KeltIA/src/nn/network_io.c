// network_io.c - SIN POOLING
#include "../../include/nn/include_nn.h"
#include <stdio.h>
#include <string.h>

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

ErrorCode save_conv_layer(FILE *f, const ConvLayer *conv)
{
    // Write conv layer configuration
    fwrite(&conv->input_channels, sizeof(size_t), 1, f);
    fwrite(&conv->input_height, sizeof(size_t), 1, f);
    fwrite(&conv->input_width, sizeof(size_t), 1, f);
    fwrite(&conv->n_filters, sizeof(size_t), 1, f);
    fwrite(&conv->kernel_height, sizeof(size_t), 1, f);
    fwrite(&conv->kernel_width, sizeof(size_t), 1, f);
    fwrite(&conv->stride, sizeof(size_t), 1, f);
    fwrite(&conv->padding, sizeof(size_t), 1, f);

    // Write kernels
    size_t kernel_size = conv->n_filters * conv->input_channels *
                         conv->kernel_height * conv->kernel_width;
    size_t written = fwrite(conv->kernels, sizeof(double), kernel_size, f);
    if (written != kernel_size)
    {
        fprintf(stderr, "network_io.c: Error writing conv kernels\n");
        return NN_ERR_WRITE;
    }

    // Write biases
    written = fwrite(conv->bias, sizeof(double), conv->n_filters, f);
    if (written != conv->n_filters)
    {
        fprintf(stderr, "network_io.c: Error writing conv biases\n");
        return NN_ERR_WRITE;
    }

    return NN_ERR_OK;
}

ErrorCode load_conv_layer(FILE *f, ConvLayer *conv)
{
    // Read conv layer configuration
    size_t input_channels, input_height, input_width;
    size_t n_filters, kernel_height, kernel_width;
    size_t stride, padding;

    if (fread(&input_channels, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;
    if (fread(&input_height, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;
    if (fread(&input_width, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;
    if (fread(&n_filters, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;
    if (fread(&kernel_height, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;
    if (fread(&kernel_width, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;
    if (fread(&stride, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;
    if (fread(&padding, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;

    // Create conv layer with read parameters
    int err = create_conv_layer(
        conv, input_channels, input_height, input_width, n_filters,
        kernel_height, kernel_width, stride, padding
    );

    if (err != 0)
    {
        fprintf(stderr, "network_io.c: Error creating conv layer\n");
        return NN_ERR_MEMORY;
    }

    // Read kernels
    size_t kernel_size =
        n_filters * input_channels * kernel_height * kernel_width;
    size_t read = fread(conv->kernels, sizeof(double), kernel_size, f);
    if (read != kernel_size)
    {
        fprintf(stderr, "network_io.c: Error reading conv kernels\n");
        return NN_ERR_READ;
    }

    // Read biases
    read = fread(conv->bias, sizeof(double), n_filters, f);
    if (read != n_filters)
    {
        fprintf(stderr, "network_io.c: Error reading conv biases\n");
        return NN_ERR_READ;
    }

    return NN_ERR_OK;
}

ErrorCode save_nn(const char *path, const NeuronalNetwork *nn)
{
    if (!nn)
    {
        fprintf(stderr, "network_io: NeuronalNetwork pointer is null\n");
        return NN_ERR_NULL_POINTER;
    }

    if (!path)
    {
        fprintf(stderr, "network_io: path pointer is null\n");
        return NN_ERR_NULL_POINTER;
    }

    FILE *f = fopen(path, "wb");
    if (!f) return NN_ERR_FILE_OPEN;

    // Write header
    fwrite(MAGIC, MAGIC_SIZE, 1, f);

    // Write number of conv layers
    fwrite(&nn->n_conv_layers, sizeof(size_t), 1, f);

    // Write each conv layer
    for (size_t i = 0; i < nn->n_conv_layers; i++)
    {
        ErrorCode err = save_conv_layer(f, &nn->conv_layers[i]);
        if (err != NN_ERR_OK)
        {
            fclose(f);
            return err;
        }
    }

    // Write number of dense layers
    fwrite(&nn->n_layers, sizeof(size_t), 1, f);

    // Write each dense layer
    for (size_t i = 0; i < nn->n_layers; i++)
    {
        fwrite(&nn->layers[i].n_inputs, sizeof(size_t), 1, f);
        fwrite(&nn->layers[i].n_neurons, sizeof(size_t), 1, f);
        fwrite(&nn->layers[i].activation_type, sizeof(int), 1, f);

        size_t count_written = fwrite(
            nn->layers[i].bias, sizeof(double), nn->layers[i].n_neurons, f
        );

        if (count_written != nn->layers[i].n_neurons)
        {
            fclose(f);
            fprintf(stderr, "network_io.c: Error writing biases\n");
            return NN_ERR_WRITE;
        }

        count_written = fwrite(
            nn->layers[i].weights, sizeof(double),
            nn->layers[i].n_neurons * nn->layers[i].n_inputs, f
        );

        if (count_written != nn->layers[i].n_neurons * nn->layers[i].n_inputs)
        {
            fclose(f);
            fprintf(stderr, "network_io.c: Error writing weights\n");
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
    if (fread(magic, sizeof(char), MAGIC_SIZE, f) != 0) return NN_ERR_READ;

    if (memcmp(magic, MAGIC, MAGIC_SIZE) != 0)
    {
        fclose(f);
        fprintf(stderr, "network_io: file does not have correct format\n");
        return NN_ERR_FORMAT;
    }

    memset(out_nn, 0, sizeof(NeuronalNetwork));

    // Read conv layers
    if (fread(&out_nn->n_conv_layers, sizeof(size_t), 1, f) != 0)
        return NN_ERR_READ;

    if (out_nn->n_conv_layers > 0)
    {
        out_nn->conv_layers = malloc(out_nn->n_conv_layers * sizeof(ConvLayer));
        if (!out_nn->conv_layers)
        {
            fclose(f);
            return NN_ERR_MEMORY;
        }

        for (size_t i = 0; i < out_nn->n_conv_layers; i++)
        {
            ErrorCode err = load_conv_layer(f, &out_nn->conv_layers[i]);
            if (err != NN_ERR_OK)
            {
                fclose(f);
                free_nn(out_nn);
                return err;
            }
        }

        // Calculate flattened size from last conv layer
        ConvLayer *last_conv = &out_nn->conv_layers[out_nn->n_conv_layers - 1];
        out_nn->flattened_size = last_conv->n_filters *
                                 last_conv->output_height *
                                 last_conv->output_width;

        out_nn->flattened = calloc(out_nn->flattened_size, sizeof(double));
        if (!out_nn->flattened)
        {
            fclose(f);
            free_nn(out_nn);
            return NN_ERR_MEMORY;
        }
    }

    // Read dense layers
    if (fread(&out_nn->n_layers, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;

    if (out_nn->n_layers > 0)
    {
        out_nn->layers = malloc(sizeof(Layer) * out_nn->n_layers);
        if (!out_nn->layers)
        {
            fclose(f);
            free_nn(out_nn);
            return NN_ERR_MEMORY;
        }

        for (size_t i = 0; i < out_nn->n_layers; i++)
        {
            size_t n_inputs, neurons;
            int type;

            if (fread(&n_inputs, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;
            if (fread(&neurons, sizeof(size_t), 1, f) != 0) return NN_ERR_READ;
            if (fread(&type, sizeof(int), 1, f) != 0) return NN_ERR_READ;

            int err = create_layer(
                &out_nn->layers[i], n_inputs, neurons, ACTIVATION_LEAKY_RELU
            );
            if (err != 0)
            {
                fclose(f);
                free_nn(out_nn);
                return NN_ERR_MEMORY;
            }

            set_activation(&out_nn->layers[i], int_to_activation(type));

            ErrorCode err_bias = load_biases_from_fs(f, &out_nn->layers[i]);
            if (err_bias != NN_ERR_OK)
            {
                fclose(f);
                free_nn(out_nn);
                return err_bias;
            }

            ErrorCode err_weight = load_weights_from_fs(f, &out_nn->layers[i]);
            if (err_weight != NN_ERR_OK)
            {
                fclose(f);
                free_nn(out_nn);
                return err_weight;
            }
        }
    }

    fclose(f);
    return NN_ERR_OK;
}
