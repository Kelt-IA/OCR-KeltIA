// back-propagation-convolutional.c - VERSIÓN COMPLETA CORREGIDA
#include "../../include/nn/include_nn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void get_empty_conv_gradients(
    ConvLayer *conv,
    double **out_grad_kernels,
    double **out_grad_bias
)
{
    size_t kernel_size = conv->n_filters * conv->input_channels *
                         conv->kernel_height * conv->kernel_width;

    *out_grad_kernels = calloc(kernel_size, sizeof(double));
    *out_grad_bias = calloc(conv->n_filters, sizeof(double));

    if (!(*out_grad_kernels) || !(*out_grad_bias))
    {
        fprintf(stderr, "Error allocating conv gradients\n");
        exit(1);
    }
}

void backward_conv_layer(
    ConvLayer *conv,
    const double *grad_output,
    double *grad_input,
    double *grad_kernels,
    double *grad_bias
)
{
    if (!conv || !grad_output || !grad_kernels || !grad_bias)
    {
        fprintf(stderr, "backward_conv_layer: NULL pointer\n");
        return;
    }

    int out_h = conv->output_height;
    int out_w = conv->output_width;
    int in_h = conv->input_height;
    int in_w = conv->input_width;
    int K_h = conv->kernel_height;
    int K_w = conv->kernel_width;
    int stride = conv->stride;
    int pad = conv->padding;

    // 1. Compute gradient w.r.t bias
    for (size_t f = 0; f < conv->n_filters; f++)
    {
        double bias_grad = 0.0;

        for (int y = 0; y < out_h; y++)
        {
            for (int x = 0; x < out_w; x++)
            {
                size_t out_idx = f * out_h * out_w + y * out_w + x;

                // ReLU derivative: gradient flows only where output > 0
                if (conv->output[out_idx] > 0.0)
                {
                    bias_grad += grad_output[out_idx];
                }
            }
        }

        grad_bias[f] += bias_grad;
    }

    // 2. Compute gradient w.r.t kernels
    for (size_t f = 0; f < conv->n_filters; f++)
    {
        for (size_t c = 0; c < conv->input_channels; c++)
        {
            for (int k_y = 0; k_y < K_h; k_y++)
            {
                for (int k_x = 0; k_x < K_w; k_x++)
                {
                    double kernel_grad = 0.0;

                    for (int out_y = 0; out_y < out_h; out_y++)
                    {
                        for (int out_x = 0; out_x < out_w; out_x++)
                        {
                            size_t out_idx =
                                f * out_h * out_w + out_y * out_w + out_x;

                            // ReLU derivative
                            if (conv->output[out_idx] > 0.0)
                            {
                                int in_y = out_y * stride + k_y - pad;
                                int in_x = out_x * stride + k_x - pad;

                                if (in_y >= 0 && in_y < in_h && in_x >= 0 &&
                                    in_x < in_w)
                                {
                                    size_t input_idx =
                                        c * in_h * in_w + in_y * in_w + in_x;
                                    kernel_grad += grad_output[out_idx] *
                                                   conv->input_cache[input_idx];
                                }
                            }
                        }
                    }

                    size_t kernel_idx =
                        ((f * conv->input_channels + c) * K_h + k_y) * K_w +
                        k_x;
                    grad_kernels[kernel_idx] += kernel_grad;
                }
            }
        }
    }

    // 3. Compute gradient w.r.t input (only if needed)
    if (grad_input != NULL)
    {
        size_t input_size = conv->input_channels * in_h * in_w;
        memset(grad_input, 0, input_size * sizeof(double));

        for (size_t f = 0; f < conv->n_filters; f++)
        {
            for (int out_y = 0; out_y < out_h; out_y++)
            {
                for (int out_x = 0; out_x < out_w; out_x++)
                {
                    size_t out_idx = f * out_h * out_w + out_y * out_w + out_x;

                    // ReLU derivative
                    if (conv->output[out_idx] > 0.0)
                    {
                        double grad_out = grad_output[out_idx];

                        for (size_t c = 0; c < conv->input_channels; c++)
                        {
                            for (int k_y = 0; k_y < K_h; k_y++)
                            {
                                for (int k_x = 0; k_x < K_w; k_x++)
                                {
                                    int in_y = out_y * stride + k_y - pad;
                                    int in_x = out_x * stride + k_x - pad;

                                    if (in_y >= 0 && in_y < in_h && in_x >= 0 &&
                                        in_x < in_w)
                                    {
                                        size_t input_idx = c * in_h * in_w +
                                                           in_y * in_w + in_x;
                                        size_t kernel_idx =
                                            ((f * conv->input_channels + c) *
                                                 K_h +
                                             k_y) *
                                                K_w +
                                            k_x;
                                        grad_input[input_idx] +=
                                            grad_out *
                                            conv->kernels[kernel_idx];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void update_conv_parameters(
    ConvLayer *conv,
    double *grad_kernels,
    double *grad_bias,
    double learning_rate
)
{
    size_t kernel_size = conv->n_filters * conv->input_channels *
                         conv->kernel_height * conv->kernel_width;

    for (size_t i = 0; i < kernel_size; i++)
    {
        conv->kernels[i] -= learning_rate * grad_kernels[i];
    }

    for (size_t i = 0; i < conv->n_filters; i++)
    {
        conv->bias[i] -= learning_rate * grad_bias[i];
    }
}
