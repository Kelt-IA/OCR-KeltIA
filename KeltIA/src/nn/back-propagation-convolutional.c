#include "../../include/nn/back-propagation-convolutional.h"
#include "../../include/nn/convolution.h"
#include "../../include/nn/include_nn.h"
#include <cblas.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Col2im: inverse of im2col, accumulates columns back to image format
void col2im(
    const double *col_buffer,
    size_t channels,
    size_t height,
    size_t width,
    size_t kernel_h,
    size_t kernel_w,
    size_t stride,
    size_t padding,
    double *output
)
{
    size_t output_h = (height + 2 * padding - kernel_h) / stride + 1;
    size_t output_w = (width + 2 * padding - kernel_w) / stride + 1;

    // Initialize output to zero
    memset(output, 0, channels * height * width * sizeof(double));

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
                            size_t output_idx =
                                c * height * width + h_idx * width + w_idx;
                            output[output_idx] += col_buffer[col_idx];
                        }
                        col_idx++;
                    }
                }
            }
        }
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
    size_t M = conv->n_filters;
    size_t K = conv->input_channels * conv->kernel_height * conv->kernel_width;
    size_t N = conv->output_height * conv->output_width;

    // OPTIMIZATION 1: Compute gradient w.r.t bias using CBLAS
    // grad_bias[f] = sum of all grad_output[f,:,:]
    // This is equivalent to: grad_bias = grad_output * ones_vector

    double *ones = malloc(N * sizeof(double));
    if (!ones)
    {
        fprintf(
            stderr,
            "back-propagation-convolutional.c: Error allocating ones vector\n"
        );
        return;
    }

    for (size_t i = 0; i < N; i++) { ones[i] = 1.0; }

    // grad_bias += grad_output * ones
    // grad_output: [M x N], ones: [N], result: [M]
    cblas_dgemv(
        CblasRowMajor, CblasNoTrans, M, N, 1.0, grad_output, N, ones, 1, 1.0,
        grad_bias, 1
    );

    free(ones);

    // Compute gradient w.r.t kernels using GEMM
    // grad_kernels = grad_output * col_buffer^T
    cblas_dgemm(
        CblasRowMajor, CblasNoTrans, CblasTrans, M, K, N, 1.0, grad_output, N,
        conv->col_buffer, N, 1.0, grad_kernels, K
    );

    // Compute gradient w.r.t input using GEMM
    // grad_col = kernels^T * grad_output
    double *grad_col = calloc(K * N, sizeof(double));
    if (!grad_col)
    {
        fprintf(
            stderr,
            "back-propagation-convolutional.c: Error allocating grad_col\n"
        );
        return;
    }

    cblas_dgemm(
        CblasRowMajor, CblasTrans, CblasNoTrans, K, N, M, 1.0, conv->kernels, K,
        grad_output, N, 0.0, grad_col, N
    );

    // Transform grad_col back to image format using col2im
    col2im(
        grad_col, conv->input_channels, conv->input_height, conv->input_width,
        conv->kernel_height, conv->kernel_width, conv->stride, conv->padding,
        grad_input
    );

    free(grad_col);
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

    // Update kernels: kernels -= learning_rate * grad_kernels
    cblas_daxpy(kernel_size, -learning_rate, grad_kernels, 1, conv->kernels, 1);

    // Update bias: bias -= learning_rate * grad_bias
    cblas_daxpy(conv->n_filters, -learning_rate, grad_bias, 1, conv->bias, 1);
}

void get_empty_conv_gradients(
    ConvLayer *conv,
    double **grad_kernels,
    double **grad_bias
)
{
    size_t kernel_size = conv->n_filters * conv->input_channels *
                         conv->kernel_height * conv->kernel_width;

    *grad_kernels = calloc(kernel_size, sizeof(double));
    *grad_bias = calloc(conv->n_filters, sizeof(double));

    if (!(*grad_kernels) || !(*grad_bias))
    {
        fprintf(
            stderr,
            "back-propagation-convolutional.c: Error allocating gradients\n"
        );
        if (*grad_kernels) free(*grad_kernels);
        if (*grad_bias) free(*grad_bias);
        exit(1);
    }
}
