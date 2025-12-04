// pooling.c - MaxPooling implementation
#include "../../include/nn/pooling.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_pool_layer(
    PoolLayer *pool,
    size_t input_channels,
    size_t input_height,
    size_t input_width,
    size_t pool_size,
    size_t stride
)
{
    memset(pool, 0, sizeof(PoolLayer));

    pool->input_channels = input_channels;
    pool->input_height = input_height;
    pool->input_width = input_width;
    pool->pool_size = pool_size;
    pool->stride = stride;

    pool->output_height = (input_height - pool_size) / stride + 1;
    pool->output_width = (input_width - pool_size) / stride + 1;

    size_t output_size =
        input_channels * pool->output_height * pool->output_width;
    pool->output = calloc(output_size, sizeof(double));
    pool->max_indices = calloc(output_size, sizeof(size_t));

    if (!pool->output || !pool->max_indices)
    {
        fprintf(stderr, "pooling.c: Error allocating memory\n");
        free_pool_layer(pool);
        return 1;
    }

    return 0;
}

void forward_pool_layer(PoolLayer *pool, const double *input)
{
    size_t out_idx = 0;

    for (size_t c = 0; c < pool->input_channels; c++)
    {
        for (size_t h = 0; h < pool->output_height; h++)
        {
            for (size_t w = 0; w < pool->output_width; w++)
            {
                double max_val = -1e9;
                size_t max_index = 0;

                // Find max value in pool window
                for (size_t ph = 0; ph < pool->pool_size; ph++)
                {
                    for (size_t pw = 0; pw < pool->pool_size; pw++)
                    {
                        size_t h_idx = h * pool->stride + ph;
                        size_t w_idx = w * pool->stride + pw;

                        if (h_idx < pool->input_height &&
                            w_idx < pool->input_width)
                        {
                            size_t input_idx =
                                c * pool->input_height * pool->input_width +
                                h_idx * pool->input_width + w_idx;

                            if (input[input_idx] > max_val)
                            {
                                max_val = input[input_idx];
                                max_index = input_idx;
                            }
                        }
                    }
                }

                pool->output[out_idx] = max_val;
                pool->max_indices[out_idx] = max_index;
                out_idx++;
            }
        }
    }
}

void backward_pool_layer(
    PoolLayer *pool,
    const double *grad_output,
    double *grad_input
)
{
    // Zero out grad_input
    size_t input_size =
        pool->input_channels * pool->input_height * pool->input_width;
    memset(grad_input, 0, input_size * sizeof(double));

    // Distribute gradients to max positions
    size_t output_size =
        pool->input_channels * pool->output_height * pool->output_width;
    for (size_t i = 0; i < output_size; i++)
    {
        grad_input[pool->max_indices[i]] += grad_output[i];
    }
}

void free_pool_layer(PoolLayer *pool)
{
    if (!pool) return;
    if (pool->output) free(pool->output);
    if (pool->max_indices) free(pool->max_indices);
    memset(pool, 0, sizeof(PoolLayer));
}
