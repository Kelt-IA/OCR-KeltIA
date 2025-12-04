// convolution.c - VERSIÓN DIRECTA (sin im2col, sin CBLAS)
#include "../../include/nn/include_nn.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void free_conv_layer(ConvLayer *conv)
{
    if (!conv) return;
    if (conv->kernels) free(conv->kernels);
    if (conv->bias) free(conv->bias);
    if (conv->output) free(conv->output);
    if (conv->input_cache) free(conv->input_cache);
    memset(conv, 0, sizeof(ConvLayer));
}

int create_conv_layer(
    ConvLayer *conv,
    size_t input_channels,
    size_t input_height,
    size_t input_width,
    size_t n_filters,
    size_t kernel_height,
    size_t kernel_width,
    size_t stride,
    size_t padding
)
{
    memset(conv, 0, sizeof(ConvLayer));

    conv->input_channels = input_channels;
    conv->input_height = input_height;
    conv->input_width = input_width;
    conv->n_filters = n_filters;
    conv->kernel_height = kernel_height;
    conv->kernel_width = kernel_width;
    conv->stride = stride;
    conv->padding = padding;

    // Calculate output dimensions
    conv->output_height =
        (input_height + 2 * padding - kernel_height) / stride + 1;
    conv->output_width =
        (input_width + 2 * padding - kernel_width) / stride + 1;

    // Allocate kernels:
    // [n_filters][input_channels][kernel_height][kernel_width]
    size_t kernel_size =
        n_filters * input_channels * kernel_height * kernel_width;
    conv->kernels = calloc(kernel_size, sizeof(double));

    // Allocate bias: [n_filters]
    conv->bias = calloc(n_filters, sizeof(double));

    // Allocate output: [n_filters][output_height][output_width]
    size_t output_size = n_filters * conv->output_height * conv->output_width;
    conv->output = calloc(output_size, sizeof(double));

    // Allocate cache for input (needed for backprop)
    size_t input_size = input_channels * input_height * input_width;
    conv->input_cache = calloc(input_size, sizeof(double));

    if (!conv->kernels || !conv->bias || !conv->output || !conv->input_cache)
    {
        fprintf(stderr, "convolution.c: Error allocating memory\n");
        free_conv_layer(conv);
        return 1;
    }

    init_conv_weights(conv);
    return 0;
}

void init_conv_weights(ConvLayer *conv)
{
    size_t fan_in =
        conv->input_channels * conv->kernel_height * conv->kernel_width;

    // He initialization for ReLU: std = sqrt(2 / fan_in)
    double he_std = sqrt(2.0 / fan_in);

    size_t kernel_size = conv->n_filters * fan_in;

    for (size_t i = 0; i < kernel_size; i++)
    {
        double u1 = ((double)rand() / RAND_MAX);
        double u2 = ((double)rand() / RAND_MAX);
        double z = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
        conv->kernels[i] = z * he_std;
    }
}

// FORWARD CONVOLUTION - Implementación directa (SIN im2col)
void forward_conv_layer(ConvLayer *conv, const double *input)
{
    // Cache input for backprop
    size_t input_size =
        conv->input_channels * conv->input_height * conv->input_width;
    memcpy(conv->input_cache, input, input_size * sizeof(double));

    int out_h = conv->output_height;
    int out_w = conv->output_width;
    int in_h = conv->input_height;
    int in_w = conv->input_width;
    int K_h = conv->kernel_height;
    int K_w = conv->kernel_width;
    int stride = conv->stride;
    int pad = conv->padding;

    // Para cada filter de salida
    for (size_t f = 0; f < conv->n_filters; f++)
    {
        // Para cada posición en el output
        for (int out_y = 0; out_y < out_h; out_y++)
        {
            for (int out_x = 0; out_x < out_w; out_x++)
            {
                double sum = conv->bias[f];

                // Para cada canal de entrada
                for (size_t c = 0; c < conv->input_channels; c++)
                {
                    // Convolve el kernel sobre esta región
                    for (int k_y = 0; k_y < K_h; k_y++)
                    {
                        for (int k_x = 0; k_x < K_w; k_x++)
                        {
                            // Posición en la imagen de entrada
                            int in_y = out_y * stride + k_y - pad;
                            int in_x = out_x * stride + k_x - pad;

                            // Check bounds (padding)
                            if (in_y >= 0 && in_y < in_h && in_x >= 0 &&
                                in_x < in_w)
                            {
                                size_t input_idx =
                                    c * in_h * in_w + in_y * in_w + in_x;
                                size_t kernel_idx =
                                    ((f * conv->input_channels + c) * K_h +
                                     k_y) *
                                        K_w +
                                    k_x;
                                sum += input[input_idx] *
                                       conv->kernels[kernel_idx];
                            }
                        }
                    }
                }

                // Escribir output con ReLU
                size_t out_idx = f * out_h * out_w + out_y * out_w + out_x;
                conv->output[out_idx] = (sum > 0.0) ? sum : 0.0;  // ReLU inline
            }
        }
    }
}
