#pragma once

#include "convolution.h"
#include <stddef.h>

// Col2im transformation (inverse of im2col)
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
);

// Backpropagation
void backward_conv_layer(
    ConvLayer *conv,
    const double *grad_output,
    double *grad_input,
    double *grad_kernels,
    double *grad_bias
);

// Parameter update
void update_conv_parameters(
    ConvLayer *conv,
    double *grad_kernels,
    double *grad_bias,
    double learning_rate
);

// Gradient allocation
void get_empty_conv_gradients(
    ConvLayer *conv,
    double **grad_kernels,
    double **grad_bias
);
