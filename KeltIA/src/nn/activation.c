#include "../../include/nn/include_nn.h"

// Sigmoid
double sigmoid(double z) { return 1.0 / (1.0 + exp(-z)); }

double sigmoid_derivative(double z, double a)
{
    (void)z;  // for unsued warning

    // 'a' is sigmoid(z)
    return a * (1.0 - a);
}

// to reduce the chance of learning rate of 0
double leaky_relu(double z) { return (z > 0) ? z : 0.01 * z; }

double leaky_relu_derivative(double z, double a)
{
    (void)a;  // for unsued warning

    return (z > 0) ? 1.0 : 0.01;
}

// perceptron
double step(double x) { return x >= 0 ? 1.0 : 0.0; }

ActivationType int_to_activation(int value)
{
    switch (value)
    {
    case ACTIVATION_SIGMOID:
        return ACTIVATION_SIGMOID;
    case ACTIVATION_LEAKY_RELU:
        return ACTIVATION_LEAKY_RELU;
    case ACTIVATION_SOFTMAX:
        return ACTIVATION_SOFTMAX;
    case ACTIVATION_STEP:
        return ACTIVATION_STEP;
    default:
        fprintf(
            stderr,
            "Warning: Invalid activation type %d, defaulting to SIGMOID\n",
            value
        );
        return ACTIVATION_SIGMOID;  // Fallback
    }
}

void softmax_activation(double *z, double *output, size_t size)
{
    // Find max for numerical stability
    double max_val = z[0];
    for (size_t i = 1; i < size; i++)
    {
        if (z[i] > max_val) max_val = z[i];
    }

    // Compute exp(z - max) and sum
    double sum = 0.0;
    for (size_t i = 0; i < size; i++)
    {
        output[i] = exp(z[i] - max_val);
        sum += output[i];
    }

    // Normalize
    for (size_t i = 0; i < size; i++) { output[i] /= sum; }
}

double softmax_derivative(double z, double output)
{
    (void)z;
    (void)output;
    return 1.0;  // For softmax + cross-entropy
}
