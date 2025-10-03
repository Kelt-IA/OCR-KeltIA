#include "../../include/nn/include_nn.h"

double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }

double sigmoid_derivative(double x)
{
    double s = sigmoid(x);
    return s * (1 - s);
}

double relu(double x) { return x > 0 ? x : 0; }
double relu_derivative(double x) { return x > 0 ? 1 : 0; }

// perceptron
double step(double x) { return x >= 0 ? 1.0 : 0.0; }
