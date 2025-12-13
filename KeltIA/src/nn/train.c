// train.c - SGD PURO (batch_size = 1)
#include "../../include/nn/include_nn.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

volatile sig_atomic_t stop_requested = 0;

void global_sigint_handler(int sig)
{
    (void)sig;
    stop_requested = 1;
    printf("\n\nInterruption received. Stopping training...\n");
}

void train_nn(
    NeuronalNetwork *nn,
    Dataset *dataset,
    size_t epochs,
    size_t batch_size  // IGNORADO, siempre usamos 1
)
{
    (void)batch_size;  // No usado

    double **deltas;
    double **grad_weights;
    double **grad_biases;

    get_empty_deltas(nn, &deltas);
    get_empty_gradients(nn, &grad_weights, &grad_biases);

    // Conv gradients
    double **conv_grad_kernels = NULL;
    double **conv_grad_bias = NULL;

    if (nn->n_conv_layers > 0)
    {
        conv_grad_kernels = malloc(nn->n_conv_layers * sizeof(double *));
        conv_grad_bias = malloc(nn->n_conv_layers * sizeof(double *));

        for (size_t i = 0; i < nn->n_conv_layers; i++)
        {
            get_empty_conv_gradients(
                &nn->conv_layers[i], &conv_grad_kernels[i], &conv_grad_bias[i]
            );
        }
    }

    for (size_t epoch = 0; epoch < epochs && !stop_requested; epoch++)
    {
        // Shuffle dataset (opcional pero recomendado)
        // TODO: implementar shuffle

        for (size_t i = 0; i < (size_t)dataset->num_samples && !stop_requested;
             i++)
        {
            // 1. BACKPROPAGATION (calcula gradientes)
            backpropagation(
                nn, dataset->inputs[i], dataset->targets[i], deltas,
                grad_weights, grad_biases, conv_grad_kernels, conv_grad_bias
            );

            // 2. UPDATE PARAMETERS INMEDIATAMENTE (SGD puro)

            // Update conv layers
            for (size_t l = 0; l < nn->n_conv_layers; l++)
            {
                update_conv_parameters(
                    &nn->conv_layers[l], conv_grad_kernels[l],
                    conv_grad_bias[l], nn->learning_rate
                );

                // Reset gradients
                size_t kernel_size = nn->conv_layers[l].n_filters *
                                     nn->conv_layers[l].input_channels *
                                     nn->conv_layers[l].kernel_height *
                                     nn->conv_layers[l].kernel_width;
                memset(conv_grad_kernels[l], 0, kernel_size * sizeof(double));
                memset(
                    conv_grad_bias[l], 0,
                    nn->conv_layers[l].n_filters * sizeof(double)
                );
            }

            // Update dense layers
            for (size_t l = 0; l < nn->n_layers; l++)
            {
                update_parameters(
                    &nn->layers[l], grad_weights[l], grad_biases[l],
                    nn->learning_rate
                );

                // Reset gradients
                size_t weight_size =
                    nn->layers[l].n_neurons * nn->layers[l].n_inputs;
                memset(grad_weights[l], 0, weight_size * sizeof(double));
                memset(
                    grad_biases[l], 0, nn->layers[l].n_neurons * sizeof(double)
                );
            }
        }
    }

    // Cleanup
    for (size_t i = 0; i < nn->n_layers; i++)
    {
        free(deltas[i]);
        free(grad_weights[i]);
        free(grad_biases[i]);
    }
    free(deltas);
    free(grad_weights);
    free(grad_biases);

    if (nn->n_conv_layers > 0)
    {
        for (size_t i = 0; i < nn->n_conv_layers; i++)
        {
            free(conv_grad_kernels[i]);
            free(conv_grad_bias[i]);
        }
        free(conv_grad_kernels);
        free(conv_grad_bias);
    }
}

void free_dataset(Dataset *dataset)
{
    if (!dataset) return;

    for (int i = 0; i < dataset->num_samples; i++)
    {
        if (dataset->inputs[i]) free(dataset->inputs[i]);
        if (dataset->targets[i]) free(dataset->targets[i]);
    }

    if (dataset->inputs) free(dataset->inputs);
    if (dataset->targets) free(dataset->targets);
    free(dataset);
}
