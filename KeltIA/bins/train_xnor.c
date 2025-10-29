#include "../include/nn/include_nn.h"
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Usage: ./train_xnor <epochs> <steps>\n");
        printf("where:\n");
        printf(
            "  <epochs>  - Number of training cycles for the neural network. "
            "Defines how many times the network will go through the entire "
            "dataset.\nIt must be grater than 0\n"
        );
        printf(
            "  <steps>   - Number of steps per epoch. Used to track the "
            "training process and print updates about how the neural network "
            "is learning.\nIt must be greater than 0\n"
        );
        return EXIT_FAILURE;
    }

    // total epochs, steps
    int epochs = atoi(argv[1]);
    int steps = atoi(argv[1]);

    // epochs = 1000
    // steps = 100
    //
    // cycles to train() = (epochs / steps) + remainder

    if (epochs <= 0 || steps <= 0)
    {
        printf("Invalid arguments were provided\n");
    }

    double training_data[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};

    // first neuron = 0, second neuron = 1
    double expected[4][2] = {{0, 1}, {1, 0}, {1, 0}, {0, 1}};  // { 1, 0, 0, 1 }

    size_t num_neuron_l[3] = {2, 4, 2};

    NeuronalNetwork nn;
    create_nn(2, 3, num_neuron_l, &nn);

    double *expected_new[4];
    double *inputs_new[4];

    for (int i = 0; i < 4; i++)
    {
        inputs_new[i] = training_data[i];
        expected_new[i] = expected[i];
    }

    Dataset dataset;
    dataset.targets = expected_new;
    dataset.inputs = inputs_new;
    dataset.input_size = 2;
    dataset.output_size = 1;
    dataset.num_samples = 4;

    int remainder = epochs % steps;
    int cycles = epochs / steps;
    EvaluationMetrics metrics;

    for (size_t i = 0; i <= cycles; i++)
    {
        metrics = evaluate_network(&nn, &dataset);
        print_evaluation(metrics, dataset.num_samples, "XNOR", i * steps);
        print_xnor_nn_predictions(&nn);

        train_nn(&nn, &dataset, steps, 0);
    }

    if (remainder != 0) { train_nn(&nn, &dataset, remainder, 0); }

    metrics = evaluate_network(&nn, &dataset);
    print_evaluation(metrics, dataset.num_samples, "XNOR", epochs);
    print_xnor_nn_predictions(&nn);

    free_nn(&nn);

    return EXIT_SUCCESS;
}
