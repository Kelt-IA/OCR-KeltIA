#include "nn.h"
#include <stdio.h>

NeuronalNetwork *xor_nn()
{
    // XOR 2->2->1
    size_t num_neuron_l[2] = {2, 1};
    NeuronalNetwork *nn = create_nn(2, 2, num_neuron_l);

    // hand crafted weights and
    double first_w[4] = {1, 1, 1, 1};
    double first_b[2] = {-0.5, -1.5};
    double second_w[2] = {1, -1};
    double second_b[1] = {-0.5};

    load_weights(&nn->layers[0], first_w);
    load_biases(&nn->layers[0], first_b);
    load_weights(&nn->layers[1], second_w);
    load_biases(&nn->layers[1], second_b);

    return nn;
}

int main()
{
    NeuronalNetwork *nn = xor_nn();

    // Dataset XOR: [x1, x2, expected_output]
    const int TESTS = 4;
    double dataset[4][3] = {{0, 0, 0}, {0, 1, 1}, {1, 0, 1}, {1, 1, 0}};

    double input[2];
    double output[1];

    for (int i = 0; i < TESTS; i++)
    {
        input[0] = dataset[i][0];
        input[1] = dataset[i][1];
        compute_nn(nn, input, output, step);
        printf("%.0f XOR %.0f = %.0f (expected %.0f)\n", input[0], input[1],
               output[0], dataset[i][2]);
    }

    free_nn(nn);
    return 0;
}
