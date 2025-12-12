#pragma once

#include <stddef.h>

typedef struct
{
    size_t input_channels;
    size_t input_height;
    size_t input_width;

    size_t n_filters;
    size_t kernel_height;
    size_t kernel_width;

    size_t stride;
    size_t padding;

    size_t output_height;
    size_t output_width;

    double
        *kernels;    // [n_filters][input_channels][kernel_height][kernel_width]
    double *bias;    // [n_filters]
    double *output;  // [n_filters][output_height][output_width]

    double *input_cache;  // cached input for backprop
    double *col_buffer;   // im2col buffer for GEMM optimization
} ConvLayer;

// Layer management
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
);

void free_conv_layer(ConvLayer *conv);

void init_conv_weights(ConvLayer *conv);

// Forward pass
void forward_conv_layer(ConvLayer *conv, const double *input);

void apply_activation_conv(ConvLayer *conv, double (*activation_fn)(double));

// Im2col transformation
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
);
