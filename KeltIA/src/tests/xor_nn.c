#include "../../include/nn/include_nn.h"
#include "../../include/tests/include_tests.h"

int test_xor_nn()
{
    NeuronalNetwork nn;
    xor_nn_perceptron(&nn);

    // Dataset XOR: [x1, x2, expected_output]
    double dataset[4][3] = {{0, 0, 0}, {0, 1, 1}, {1, 0, 1}, {1, 1, 0}};

    double input[2];
    double output[1];
    int all_correct = 1;

    printf("\n  Testing XOR function:\n");

    for (int i = 0; i < 4; i++)
    {
        input[0] = dataset[i][0];
        input[1] = dataset[i][1];

        compute_nn(&nn, input, output);

        double expected = dataset[i][2];
        int correct = (output[0] == expected);

        if (correct) { printf(COLOR_GREEN "    ✓ " COLOR_RESET); }
        else
        {
            printf(COLOR_RED "    ✗ " COLOR_RESET);
            all_correct = 0;
        }

        printf(
            "%.0f XOR %.0f = %.2f (expected %.0f)\n", input[0], input[1],
            output[0], expected
        );
    }

    free_nn(&nn);

    return all_correct;
}
