// test_pooling.c - Comprehensive PoolLayer tests
#include "../include/nn/include_nn.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(condition, message)                                             \
    do                                                                         \
    {                                                                          \
        if (!(condition))                                                      \
        {                                                                      \
            fprintf(                                                           \
                stderr, "❌ FAIL: %s\n   at %s:%d\n", message, __FILE__,       \
                __LINE__                                                       \
            );                                                                 \
            return 1;                                                          \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            printf("✅ PASS: %s\n", message);                                  \
        }                                                                      \
    } while (0)

#define ASSERT_NEAR(a, b, epsilon, message)                                    \
    do                                                                         \
    {                                                                          \
        if (fabs((a) - (b)) > (epsilon))                                       \
        {                                                                      \
            fprintf(                                                           \
                stderr,                                                        \
                "❌ FAIL: %s\n   Expected %.6f, got %.6f (diff: %.6f)\n   at " \
                "%s:%d\n",                                                     \
                message, (double)(b), (double)(a), fabs((a) - (b)), __FILE__,  \
                __LINE__                                                       \
            );                                                                 \
            return 1;                                                          \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            printf(                                                            \
                "✅ PASS: %s (%.6f ≈ %.6f)\n", message, (double)(a),           \
                (double)(b)                                                    \
            );                                                                 \
        }                                                                      \
    } while (0)

int test_pool_creation()
{
    printf("\n=== Test 1: PoolLayer Creation ===\n");

    PoolLayer pool;
    int err = create_pool_layer(&pool, 16, 32, 32, 2, 2);

    ASSERT(err == 0, "PoolLayer created successfully");
    ASSERT(pool.input_channels == 16, "Input channels = 16");
    ASSERT(pool.input_height == 32, "Input height = 32");
    ASSERT(pool.input_width == 32, "Input width = 32");
    ASSERT(pool.pool_size == 2, "Pool size = 2");
    ASSERT(pool.stride == 2, "Stride = 2");

    // Output size: (32 - 2) / 2 + 1 = 16
    ASSERT(pool.output_height == 16, "Output height = 16");
    ASSERT(pool.output_width == 16, "Output width = 16");

    ASSERT(pool.output != NULL, "Output allocated");
    ASSERT(pool.max_indices != NULL, "Max indices allocated");

    free_pool_layer(&pool);
    return 0;
}

int test_pool_forward_simple()
{
    printf("\n=== Test 2: Forward Pass Simple ===\n");

    PoolLayer pool;
    create_pool_layer(&pool, 1, 4, 4, 2, 2);

    // Input: 4x4 with known values
    // 1  2  | 3  4
    // 5  6  | 7  8
    // -----------
    // 9  10 | 11 12
    // 13 14 | 15 16
    double input[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    forward_pool_layer(&pool, input);

    // Expected output (2x2):
    // max(1,2,5,6)=6    max(3,4,7,8)=8
    // max(9,10,13,14)=14  max(11,12,15,16)=16

    ASSERT(pool.output_height == 2, "Output height = 2");
    ASSERT(pool.output_width == 2, "Output width = 2");

    ASSERT_NEAR(
        pool.output[0], 6.0, 0.001, "Output[0,0] = 6 (max of top-left)"
    );
    ASSERT_NEAR(
        pool.output[1], 8.0, 0.001, "Output[0,1] = 8 (max of top-right)"
    );
    ASSERT_NEAR(
        pool.output[2], 14.0, 0.001, "Output[1,0] = 14 (max of bottom-left)"
    );
    ASSERT_NEAR(
        pool.output[3], 16.0, 0.001, "Output[1,1] = 16 (max of bottom-right)"
    );

    free_pool_layer(&pool);
    return 0;
}

int test_pool_max_indices()
{
    printf("\n=== Test 3: Max Indices Tracking ===\n");

    PoolLayer pool;
    create_pool_layer(&pool, 1, 4, 4, 2, 2);

    double input[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    forward_pool_layer(&pool, input);

    // Check that indices are stored correctly
    // output[0] = 6, which is at input index 5
    printf(
        "  Max index for output[0] = %zu (value %.0f at input[5])\n",
        pool.max_indices[0], input[pool.max_indices[0]]
    );
    ASSERT(pool.max_indices[0] == 5, "Max index[0] = 5 (position of value 6)");

    // output[1] = 8, which is at input index 7
    printf(
        "  Max index for output[1] = %zu (value %.0f at input[7])\n",
        pool.max_indices[1], input[pool.max_indices[1]]
    );
    ASSERT(pool.max_indices[1] == 7, "Max index[1] = 7 (position of value 8)");

    // output[2] = 14, which is at input index 13
    printf(
        "  Max index for output[2] = %zu (value %.0f at input[13])\n",
        pool.max_indices[2], input[pool.max_indices[2]]
    );
    ASSERT(
        pool.max_indices[2] == 13, "Max index[2] = 13 (position of value 14)"
    );

    // output[3] = 16, which is at input index 15
    printf(
        "  Max index for output[3] = %zu (value %.0f at input[15])\n",
        pool.max_indices[3], input[pool.max_indices[3]]
    );
    ASSERT(
        pool.max_indices[3] == 15, "Max index[3] = 15 (position of value 16)"
    );

    free_pool_layer(&pool);
    return 0;
}

int test_pool_backward()
{
    printf("\n=== Test 4: Backward Pass ===\n");

    PoolLayer pool;
    create_pool_layer(&pool, 1, 4, 4, 2, 2);

    double input[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

    forward_pool_layer(&pool, input);

    // Gradient from next layer
    double grad_output[4] = {1.0, 2.0, 3.0, 4.0};

    double grad_input[16];
    backward_pool_layer(&pool, grad_output, grad_input);

    // Gradients should only go to max positions
    // grad_input[5] = 1.0 (from output[0])
    // grad_input[7] = 2.0 (from output[1])
    // grad_input[13] = 3.0 (from output[2])
    // grad_input[15] = 4.0 (from output[3])
    // All others = 0

    printf("  Gradient values:\n");
    for (int i = 0; i < 16; i++)
    {
        if (grad_input[i] != 0.0)
        {
            printf("    grad_input[%d] = %.1f\n", i, grad_input[i]);
        }
    }

    ASSERT_NEAR(
        grad_input[5], 1.0, 0.001, "Gradient at max position [5] = 1.0"
    );
    ASSERT_NEAR(
        grad_input[7], 2.0, 0.001, "Gradient at max position [7] = 2.0"
    );
    ASSERT_NEAR(
        grad_input[13], 3.0, 0.001, "Gradient at max position [13] = 3.0"
    );
    ASSERT_NEAR(
        grad_input[15], 4.0, 0.001, "Gradient at max position [15] = 4.0"
    );

    // Check that non-max positions have zero gradient
    ASSERT_NEAR(
        grad_input[0], 0.0, 0.001, "Non-max position has zero gradient"
    );
    ASSERT_NEAR(
        grad_input[6], 0.0, 0.001, "Non-max position has zero gradient"
    );

    free_pool_layer(&pool);
    return 0;
}

int test_pool_multiple_channels()
{
    printf("\n=== Test 5: Multiple Channels ===\n");

    PoolLayer pool;
    create_pool_layer(&pool, 3, 4, 4, 2, 2);  // 3 channels

    ASSERT(pool.input_channels == 3, "Input channels = 3");

    // Input: 3 channels * 4x4 = 48 values
    double input[48];
    for (int i = 0; i < 48; i++) input[i] = (double)i;

    forward_pool_layer(&pool, input);

    // Output: 3 channels * 2x2 = 12 values
    size_t output_size = 3 * 2 * 2;
    ASSERT(output_size == 12, "Output size = 12");

    // Check that each channel is processed independently
    // Channel 0: values 0-15, max in first window should be 5 (positions 0-5 in
    // pattern)
    printf("  Channel 0, output[0] = %.0f\n", pool.output[0]);
    ASSERT_NEAR(pool.output[0], 5.0, 0.001, "Channel 0 processed correctly");

    // Channel 1: values 16-31, max in first window should be 21
    printf("  Channel 1, output[4] = %.0f\n", pool.output[4]);
    ASSERT_NEAR(pool.output[4], 21.0, 0.001, "Channel 1 processed correctly");

    // Channel 2: values 32-47, max in first window should be 37
    printf("  Channel 2, output[8] = %.0f\n", pool.output[8]);
    ASSERT_NEAR(pool.output[8], 37.0, 0.001, "Channel 2 processed correctly");

    free_pool_layer(&pool);
    return 0;
}

int test_pool_with_negatives()
{
    printf("\n=== Test 6: Negative Values ===\n");

    PoolLayer pool;
    create_pool_layer(&pool, 1, 4, 4, 2, 2);

    // Input with negative values
    double input[16] = {-10, -5,  -3, -2, -8,  -4,  -1, 0,
                        -15, -12, -6, -3, -20, -18, -9, -5};

    forward_pool_layer(&pool, input);

    // Max of each 2x2 block (least negative = max)
    // Top-left: max(-10,-5,-8,-4) = -4
    // Top-right: max(-3,-2,-1,0) = 0
    // Bottom-left: max(-15,-12,-20,-18) = -12
    // Bottom-right: max(-6,-3,-9,-5) = -3

    ASSERT_NEAR(pool.output[0], -4.0, 0.001, "Max of negative values = -4");
    ASSERT_NEAR(pool.output[1], 0.0, 0.001, "Max includes zero = 0");
    ASSERT_NEAR(pool.output[2], -12.0, 0.001, "Max of all negative = -12");
    ASSERT_NEAR(pool.output[3], -3.0, 0.001, "Max of negative values = -3");

    free_pool_layer(&pool);
    return 0;
}

int test_pool_stride_variations()
{
    printf("\n=== Test 7: Different Stride Values ===\n");

    PoolLayer pool1, pool2;
    create_pool_layer(&pool1, 1, 8, 8, 2, 1);  // stride=1, overlapping
    create_pool_layer(&pool2, 1, 8, 8, 2, 2);  // stride=2, non-overlapping

    // stride=1: (8 - 2) / 1 + 1 = 7
    ASSERT(pool1.output_height == 7, "Stride 1: output height = 7");
    ASSERT(pool1.output_width == 7, "Stride 1: output width = 7");

    // stride=2: (8 - 2) / 2 + 1 = 4
    ASSERT(pool2.output_height == 4, "Stride 2: output height = 4");
    ASSERT(pool2.output_width == 4, "Stride 2: output width = 4");

    free_pool_layer(&pool1);
    free_pool_layer(&pool2);
    return 0;
}

int test_pool_identical_values()
{
    printf("\n=== Test 8: Identical Values in Window ===\n");

    PoolLayer pool;
    create_pool_layer(&pool, 1, 4, 4, 2, 2);

    // All values are the same
    double input[16];
    for (int i = 0; i < 16; i++) input[i] = 5.0;

    forward_pool_layer(&pool, input);

    // Output should be all 5.0
    for (size_t i = 0; i < 4; i++)
    {
        ASSERT_NEAR(
            pool.output[i], 5.0, 0.001,
            "Identical values produce identical output"
        );
    }

    // Max indices should still be valid
    ASSERT(pool.max_indices[0] < 16, "Max index is valid");

    free_pool_layer(&pool);
    return 0;
}

int test_pool_full_pipeline()
{
    printf("\n=== Test 9: Full Conv+Pool Pipeline ===\n");

    ConvLayer conv;
    PoolLayer pool;

    // Conv: 8x8 -> 8x8 (with padding)
    create_conv_layer(&conv, 1, 8, 8, 4, 3, 3, 1, 1);

    // Pool: 8x8 -> 4x4
    create_pool_layer(&pool, 4, 8, 8, 2, 2);

    double input[64];
    for (int i = 0; i < 64; i++) input[i] = (double)(i % 10);

    // Forward through conv
    forward_conv_layer(&conv, input);
    printf(
        "  Conv output size: %zu x %zu x %zu\n", conv.n_filters,
        conv.output_height, conv.output_width
    );

    // Forward through pool
    forward_pool_layer(&pool, conv.output);
    printf(
        "  Pool output size: %zu x %zu x %zu\n", pool.input_channels,
        pool.output_height, pool.output_width
    );

    ASSERT(pool.output_height == 4, "Pool reduces spatial dimensions");
    ASSERT(pool.output_width == 4, "Pool reduces spatial dimensions");

    // Backward through pool
    double grad_pool_output[64];  // 4 channels * 4x4
    for (int i = 0; i < 64; i++) grad_pool_output[i] = 0.1;

    double grad_pool_input[256];  // 4 channels * 8x8
    backward_pool_layer(&pool, grad_pool_output, grad_pool_input);

    // Check that gradients are sparse (only at max positions)
    int nonzero_count = 0;
    for (int i = 0; i < 256; i++)
    {
        if (grad_pool_input[i] != 0.0) nonzero_count++;
    }

    printf("  Non-zero gradients: %d / 256\n", nonzero_count);
    ASSERT(nonzero_count == 64, "Gradients only at max positions (4x4x4 = 64)");

    free_pool_layer(&pool);
    free_conv_layer(&conv);
    return 0;
}

int test_pool_large_input()
{
    printf("\n=== Test 10: Large Input (32x32) ===\n");

    PoolLayer pool;
    create_pool_layer(&pool, 16, 32, 32, 2, 2);

    // Input: 16 channels * 32x32 = 16384 values
    double *input = malloc(16384 * sizeof(double));
    for (int i = 0; i < 16384; i++) input[i] = (double)(i % 100);

    forward_pool_layer(&pool, input);

    // Output: 16 channels * 16x16 = 4096 values
    ASSERT(pool.output_height == 16, "Output height = 16");
    ASSERT(pool.output_width == 16, "Output width = 16");

    size_t output_size = 16 * 16 * 16;
    ASSERT(output_size == 4096, "Output size = 4096");

    printf("  Successfully pooled 16x32x32 -> 16x16x16\n");

    free(input);
    free_pool_layer(&pool);
    return 0;
}

int main()
{
    printf("╔══════════════════════════════════════╗\n");
    printf("║   PoolLayer Comprehensive Tests     ║\n");
    printf("╚══════════════════════════════════════╝\n");

    int failed = 0;

    failed += test_pool_creation();
    failed += test_pool_forward_simple();
    failed += test_pool_max_indices();
    failed += test_pool_backward();
    failed += test_pool_multiple_channels();
    failed += test_pool_with_negatives();
    failed += test_pool_stride_variations();
    failed += test_pool_identical_values();
    failed += test_pool_full_pipeline();
    failed += test_pool_large_input();

    printf("\n");
    printf("═══════════════════════════════════════\n");
    if (failed == 0) { printf("✅ All %d tests PASSED!\n", 10); }
    else
    {
        printf("❌ %d test(s) FAILED!\n", failed);
    }
    printf("═══════════════════════════════════════\n");

    return failed;
}
