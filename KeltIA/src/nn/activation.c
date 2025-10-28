#include "../../include/nn/include_nn.h"

// Sigmoid
double sigmoid(double z) { return 1.0 / (1.0 + exp(-z)); }

double sigmoid_derivative(double z, double a)
{
    (void)z;  // for unsued warning

    // Usamos 'a' is sigmoid(z)
    return a * (1.0 - a);
}

// Leaky ReLU
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
    case ACTIVATION_STEP:
        return ACTIVATION_STEP;
    default:
        fprintf(
            stderr,
            "Warning: Invalid activation type %d, defaulting to SIGMOID\n",
            value
        );
        return ACTIVATION_SIGMOID;  // Fallback seguro
    }
}
