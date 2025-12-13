#pragma once

#include <stdlib.h>
double sigmoid(double z);
double sigmoid_derivative(double z, double a);
double leaky_relu(double z);
double leaky_relu_derivative(double z, double a);
double softmax_derivative(double z, double a);
double step(double x);

typedef enum
{
    ACTIVATION_SIGMOID,     // 0
    ACTIVATION_LEAKY_RELU,  // 1
    ACTIVATION_STEP,        // 2
    ACTIVATION_SOFTMAX,     // 3
} ActivationType;

typedef double (*ActivationFunction)(double a);
typedef double (*DerivativeFunction)(double z, double a);

ActivationType int_to_activation(int value);

void softmax_activation(double *z, double *output, size_t size);
double softmax_derivative(double z, double output);
