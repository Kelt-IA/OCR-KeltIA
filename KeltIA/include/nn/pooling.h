#pragma once

#include <stddef.h>

typedef struct
{
    size_t input_channels;
    size_t input_height;
    size_t input_width;
    size_t pool_size;
    size_t stride;

    size_t output_height;
    size_t output_width;

    double *output;
    size_t *max_indices;  // Store indices for backprop
} PoolLayer;

int create_pool_layer(
    PoolLayer *pool,
    size_t input_channels,
    size_t input_height,
    size_t input_width,
    size_t pool_size,
    size_t stride
);

void forward_pool_layer(PoolLayer *pool, const double *input);
void backward_pool_layer(
    PoolLayer *pool,
    const double *grad_output,
    double *grad_input
);
void free_pool_layer(PoolLayer *pool);
