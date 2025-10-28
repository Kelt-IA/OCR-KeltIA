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
