#include "../../include/nn/include_nn.h"

double *delta_output(Layer *last_layer, double *expected)
{
    double *delta = (double *)calloc(last_layer->n_neurons, sizeof(double));

    for (int i = 0; i < last_layer->n_neurons; i++)
    {
        double error = last_layer->output[i] - expected[i];
        double deriv = last_layer->output[i] * (1 - last_layer->output[i]);

        delta[i] = error * deriv;
    }

    return delta;
}

double *delta_hidden_layer(Layer *layer, Layer *next_layer, double *delta_next)
{
    double *delta = (double *)calloc(layer->n_neurons, sizeof(double));

    for (int i = 0; i < layer->n_neurons; i++)
    {
        double sum = 0;
        for (int j = 0; j < next_layer->n_neurons; j++)
        {
            sum += WEIGHT(layer, i, j) * delta_next[i];
        }

        double deriv = layer->output[i] * (1 - layer->output[i]);

        delta[i] = sum * deriv;
    }

    return delta;
}

double *gradient_weights() {}
