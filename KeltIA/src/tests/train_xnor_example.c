#include "../../include/nn/include_nn.h"

void test_xnor_nn_train(int number_of_epochs)
{
    double training_data[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};

    // first neuron = 0, second neuron = 1
    double expected[4][2] = {{0, 1}, {1, 0}, {1, 0}, {0, 1}};

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

    EvaluationMetrics metrics = evaluate_network(&nn, &dataset);
    print_evaluation(metrics, dataset.num_samples, "XNOR", 0);
    print_xnor_nn_predictions(&nn);

    train_nn(&nn, &dataset, number_of_epochs, 2);

    metrics = evaluate_network(&nn, &dataset);
    print_evaluation(metrics, dataset.num_samples, "XNOR", number_of_epochs);
    print_xnor_nn_predictions(&nn);

    free_nn(&nn);
}
