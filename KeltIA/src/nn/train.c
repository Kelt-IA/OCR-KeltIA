#include "../../include/nn/include_nn.h"
#include <stddef.h>
#include <string.h>

void average_gradients(
    NeuronalNetwork *nn,
    double **grad_weights,
    double **grad_biases,
    double batch_size
)
{
    // makes an average of the gradients, made for mini-batches

    if (batch_size == 0) return;

    for (size_t i = 0; i < nn->n_layers; i++)
    {
        Layer *layer = &nn->layers[i];

        size_t size_grad_weight = layer->n_neurons * layer->n_inputs;
        size_t size_grad_bias = layer->n_neurons;

        for (size_t w = 0; w < size_grad_weight; w++)
        {
            grad_weights[i][w] /= batch_size;
        }

        for (size_t b = 0; b < size_grad_bias; b++)
        {
            grad_biases[i][b] /= batch_size;
        }
    }
}

void reset_gradients(
    NeuronalNetwork *nn,
    double **grad_weights,
    double **grad_biases
)
{
    for (size_t i = 0; i < nn->n_layers; i++)
    {
        Layer *layer = &nn->layers[i];

        size_t size_grad_weight = layer->n_neurons * layer->n_inputs;
        size_t size_grad_bias = layer->n_neurons;

        // Pone todos los gradientes a 0
        memset(grad_weights[i], 0, size_grad_weight * sizeof(double));
        memset(grad_biases[i], 0, size_grad_bias * sizeof(double));
    }
}

void train_nn(
    NeuronalNetwork *nn,
    Dataset *dataset,
    size_t epochs,
    double batch
)
{
    double **deltas = NULL;
    double **grad_weights = NULL;
    double **grad_biases = NULL;

    get_empty_deltas(nn, &deltas);
    get_empty_gradients(nn, &grad_weights, &grad_biases);

    for (int i = 0; i <= (int)epochs; i++)
    {

        for (int j = 0; j < dataset->num_samples; j++)
        {
            backpropagation(
                nn, dataset->inputs[j], dataset->targets[j], deltas,
                grad_weights, grad_biases
            );
        }

        if (batch == 0 || i % (int)batch == 0)
        {
            // average gradiants
            average_gradients(nn, grad_weights, grad_biases, batch);

            for (size_t l = 0; l < nn->n_layers; l++)
            {

                update_parameters(
                    &nn->layers[l], grad_weights[l], grad_biases[l],
                    LEARNING_RATE
                );
            }

            // set gradiants to 0
            reset_gradients(nn, grad_weights, grad_biases);
        }
    }

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
