#pragma once

double sigmoid(double z);
double sigmoid_derivative(double z, double a);
double leaky_relu(double z);
double leaky_relu_derivative(double z, double a);
double step(double x);

typedef enum
{
    ACTIVATION_SIGMOID,     // 0
    ACTIVATION_LEAKY_RELU,  // 1
    ACTIVATION_STEP,        // 2
} ActivationType;

typedef double (*ActivationFunction)(double);
typedef double (*DerivativeFunction)(double, double);  // (z, a)

ActivationType int_to_activation(int value);
