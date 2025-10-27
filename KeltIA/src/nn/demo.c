#include "../../include/nn/include_nn.h"
#include <stdio.h>

void xor_nn(NeuronalNetwork *nn)
{
    // XOR 2->2->1
    size_t num_neuron_l[2] = {2, 1};

    create_nn(2, 2, num_neuron_l, nn, WEIGHT_INIT_SEED);

    // hand crafted weights and biases
    double first_w[4] = {1, 1, 1, 1};  // 2 neurons, 2 inputs each -> 4 weights
    double first_b[2] = {-0.5, -1.5};  // 2 neurons, 2 biases

    double second_w[2] = {1, -1};  // 1 neuron,  2 inputs -> 2 weights
    double second_b[1] = {-0.5};   // 1 neuron,  1 bias

    // layer 1 (hidden layer)
    load_weights(&nn->layers[0], first_w);
    load_biases(&nn->layers[0], first_b);

    // layer 2 (exit)
    load_weights(&nn->layers[1], second_w);
    load_biases(&nn->layers[1], second_b);
}

void xor_nn_train(int number_of_epochs)
{
    // {input 1, input 2, expected_result}
    double training_data[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    double expected[4][1] = {{0}, {1}, {1}, {0}};

    size_t num_neuron_l[2] = {2, 1};

    NeuronalNetwork nn;
    create_nn(2, 2, num_neuron_l, &nn, WEIGHT_INIT_SEED);

    double **deltas = NULL;
    get_empty_deltas(nn, deltas);

    double **grad_weights = NULL;
    double **grad_biases = NULL;
    get_empty_gradients(nn, grad_weights, grad_biases);

    for (int i = 0; i < number_of_epochs; i++)
    {
        if (i % 100 == 0)
        {
            // test nn
        }

        for (int i = 0; i < 4; i++)
        {
            backpropagation(
                &nn, training_data[i], expected[i], deltas, grad_weights,
                grad_biases
            );
        }
    }

    // frees
    for (size_t i = 0; i < nn.n_layers; i++) { free(deltas[i]); }
    free(deltas);

    for (size_t i = 0; i < nn.n_layers; i++)
    {
        free(grad_weights[i]);
        free(grad_biases[i]);
    }
    free(grad_weights);
    free(grad_biases);
}
