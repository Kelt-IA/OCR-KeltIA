#include "../../include/nn/include_nn.h"
#include <cblas.h>
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
    if (conv->col_buffer) free(conv->col_buffer);
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

    // Allocate column buffer for im2col
    size_t col_height = input_channels * kernel_height * kernel_width;
    size_t col_width = conv->output_height * conv->output_width;
    conv->col_buffer = calloc(col_height * col_width, sizeof(double));

    if (!conv->kernels || !conv->bias || !conv->output || !conv->input_cache ||
        !conv->col_buffer)
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
    size_t fan_out = conv->n_filters * conv->kernel_height * conv->kernel_width;
    double xavier_limit = sqrt(6.0 / (fan_in + fan_out));

    size_t kernel_size = conv->n_filters * conv->input_channels *
                         conv->kernel_height * conv->kernel_width;

    for (size_t i = 0; i < kernel_size; i++)
    {
        conv->kernels[i] =
            ((double)rand() / RAND_MAX) * 2.0 * xavier_limit - xavier_limit;
    }
}

// Im2col transformation: converts image patches to columns for GEMM
void im2col(
    const double *input,
    size_t channels,
    size_t height,
    size_t width,
    size_t kernel_h,
    size_t kernel_w,
    size_t stride,
    size_t padding,
    double *col_buffer
)
{
    size_t output_h = (height + 2 * padding - kernel_h) / stride + 1;
    size_t output_w = (width + 2 * padding - kernel_w) / stride + 1;

    size_t col_idx = 0;

    for (size_t c = 0; c < channels; c++)
    {
        for (size_t kh = 0; kh < kernel_h; kh++)
        {
            for (size_t kw = 0; kw < kernel_w; kw++)
            {
                for (size_t out_h = 0; out_h < output_h; out_h++)
                {
                    for (size_t out_w = 0; out_w < output_w; out_w++)
                    {
                        int h_idx = (int)(out_h * stride + kh) - (int)padding;
                        int w_idx = (int)(out_w * stride + kw) - (int)padding;

                        if (h_idx >= 0 && h_idx < (int)height && w_idx >= 0 &&
                            w_idx < (int)width)
                        {
                            size_t input_idx =
                                c * height * width + h_idx * width + w_idx;
                            col_buffer[col_idx] = input[input_idx];
                        }
                        else
                        {
                            col_buffer[col_idx] = 0.0;  // padding
                        }
                        col_idx++;
                    }
                }
            }
        }
    }
}

void forward_conv_layer(ConvLayer *conv, const double *input)
{
    // Cache input for backprop
    size_t input_size =
        conv->input_channels * conv->input_height * conv->input_width;
    memcpy(conv->input_cache, input, input_size * sizeof(double));

    // Transform input to column format
    im2col(
        input, conv->input_channels, conv->input_height, conv->input_width,
        conv->kernel_height, conv->kernel_width, conv->stride, conv->padding,
        conv->col_buffer
    );

    // Dimensions for GEMM
    size_t M = conv->n_filters;
    size_t K = conv->input_channels * conv->kernel_height * conv->kernel_width;
    size_t N = conv->output_height * conv->output_width;

    // Matrix multiplication: output = kernels * col_buffer
    cblas_dgemm(
        CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, 1.0, conv->kernels,
        K, conv->col_buffer, N, 0.0, conv->output, N
    );

    // OPTIMIZATION: Add bias using CBLAS operations
    // Create ones vector for broadcasting bias
    double *ones = malloc(N * sizeof(double));
    if (!ones)
    {
        fprintf(stderr, "convolution.c: Error allocating ones vector\n");
        return;
    }

    for (size_t i = 0; i < N; i++) { ones[i] = 1.0; }

    // Add bias using outer product: output += bias * ones^T
    // This broadcasts each bias value across all spatial positions
    cblas_dger(
        CblasRowMajor, M, N, 1.0, conv->bias, 1, ones, 1, conv->output, N
    );

    free(ones);
}

void apply_activation_conv(ConvLayer *conv, ActivationFunction activation_fn)
{
    size_t output_size =
        conv->n_filters * conv->output_height * conv->output_width;

    for (size_t i = 0; i < output_size; i++)
    {
        conv->output[i] = activation_fn(conv->output[i]);
    }
}
