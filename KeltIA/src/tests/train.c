#include "../../include/nn/include_nn.h"

void test_xor_nn_train(int number_of_epochs)
{
    double training_data[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};

    // first neuron = 0, second neuron = 1
    double expected[4][2] = {{1, 0}, {0, 1}, {0, 1}, {1, 0}};

    size_t num_neuron_l[3] = {2, 4, 2};

    NeuronalNetwork nn;
    create_nn(2, 3, num_neuron_l, &nn);

    double **deltas = NULL;
    double **grad_weights = NULL;
    double **grad_biases = NULL;

    get_empty_deltas(nn, &deltas);
    get_empty_gradients(nn, &grad_weights, &grad_biases);

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

    for (int i = 0; i <= number_of_epochs; i++)
    {
        if (i % 100 == 0)
        {
            EvaluationMetrics metrics = evaluate_network(&nn, &dataset);
            print_evaluation(metrics, dataset.num_samples, "XOR", i);
            print_xor_nn_predictions(&nn);
            // test nn
        }

        for (int i = 0; i < dataset.num_samples; i++)
        {
            backpropagation(
                &nn, training_data[i], expected[i], deltas, grad_weights,
                grad_biases
            );

            for (size_t l = 0; l < nn.n_layers; l++)
            {
                update_parameters(
                    &nn.layers[l], grad_weights[l], grad_biases[l],
                    LEARNING_RATE
                );
            }
        }
    }

    for (size_t i = 0; i < nn.n_layers; i++)
    {
        free(deltas[i]);
        free(grad_weights[i]);
        free(grad_biases[i]);
    }

    free(deltas);
    free(grad_weights);
    free(grad_biases);

    free_nn(&nn);
}
