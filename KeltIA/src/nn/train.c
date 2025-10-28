#include "../../include/nn/include_nn.h"

void train(NeuronalNetwork *nn, Dataset dataset, int batch)
{
    double **deltas = NULL;
    double **grad_weights = NULL;
    double **grad_biases = NULL;

    // tmp
    (void)batch;
    (void)dataset;

    get_empty_deltas(nn, &deltas);
    get_empty_gradients(nn, &grad_weights, &grad_biases);
}
