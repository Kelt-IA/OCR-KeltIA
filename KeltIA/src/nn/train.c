// train.c - Training functions for neural networks
#include "../../include/nn/include_nn.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    size_t batch_size
)
{
    if (!nn || !dataset)
    {
        fprintf(stderr, "train.c: Invalid neural network or dataset\n");
        return;
    }

    double **deltas;
    double **grad_weights;
    double **grad_biases;

    get_empty_deltas(nn, &deltas);
    get_empty_gradients(nn, &grad_weights, &grad_biases);

    for (size_t epoch = 0; epoch < epochs && !stop_requested; epoch++)
    {
        for (size_t i = 0; i < (size_t)dataset->num_samples && !stop_requested;
             i++)
        {
            // Backprop handles both dense and conv layers automatically
            backpropagation(
                nn, dataset->inputs[i], dataset->targets[i], deltas,
                grad_weights, grad_biases
            );

            // Update dense layer parameters every batch_size samples
            if ((i + 1) % batch_size == 0 ||
                i == (size_t)dataset->num_samples - 1)
            {
                for (size_t l = 0; l < nn->n_layers; l++)
                {
                    update_parameters(
                        &nn->layers[l], grad_weights[l], grad_biases[l],
                        LEARNING_RATE
                    );

                    // Reset gradients for next batch
                    size_t weight_size =
                        nn->layers[l].n_neurons * nn->layers[l].n_inputs;
                    memset(grad_weights[l], 0, weight_size * sizeof(double));
                    memset(
                        grad_biases[l], 0,
                        nn->layers[l].n_neurons * sizeof(double)
                    );
                }
            }
        }
    }

    // Free gradients
    for (size_t i = 0; i < nn->n_layers; i++)
    {
        free(deltas[i]);
        free(grad_weights[i]);
        free(grad_biases[i]);
    }
    free(deltas);
    free(grad_weights);
    free(grad_biases);
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
