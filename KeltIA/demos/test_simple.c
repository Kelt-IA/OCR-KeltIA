// test_conv.c - Comprehensive ConvLayer tests
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

int test_conv_creation()
{
    printf("\n=== Test 1: ConvLayer Creation ===\n");

    ConvLayer conv;
    int err = create_conv_layer(&conv, 1, 32, 32, 8, 3, 3, 1, 1);

    ASSERT(err == 0, "ConvLayer created successfully");
    ASSERT(conv.input_channels == 1, "Input channels = 1");
    ASSERT(conv.input_height == 32, "Input height = 32");
    ASSERT(conv.input_width == 32, "Input width = 32");
    ASSERT(conv.n_filters == 8, "Number of filters = 8");
    ASSERT(conv.kernel_height == 3, "Kernel height = 3");
    ASSERT(conv.kernel_width == 3, "Kernel width = 3");
    ASSERT(conv.stride == 1, "Stride = 1");
    ASSERT(conv.padding == 1, "Padding = 1");

    // Output size: (32 + 2*1 - 3) / 1 + 1 = 32
    ASSERT(conv.output_height == 32, "Output height = 32");
    ASSERT(conv.output_width == 32, "Output width = 32");

    ASSERT(conv.kernels != NULL, "Kernels allocated");
    ASSERT(conv.bias != NULL, "Bias allocated");
    ASSERT(conv.output != NULL, "Output allocated");

    free_conv_layer(&conv);
    return 0;
}

int test_conv_forward_basic()
{
    printf("\n=== Test 2: Forward Pass Basic ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 1, 4, 4, 1, 3, 3, 1, 0);

    // Set kernels to simple identity-like pattern
    for (size_t i = 0; i < 9; i++) conv.kernels[i] = 0.0;
    conv.kernels[4] = 1.0;  // Center of 3x3 kernel = 1.0
    conv.bias[0] = 0.0;

    // Input: 4x4 with all 1s
    double input[16];
    for (int i = 0; i < 16; i++) input[i] = 1.0;

    forward_conv_layer(&conv, input);

    // Output should be 2x2 (4 - 3 + 1 = 2)
    ASSERT(conv.output_height == 2, "Output height = 2");
    ASSERT(conv.output_width == 2, "Output width = 2");

    // With center kernel = 1, each output should be ~1.0
    ASSERT_NEAR(conv.output[0], 1.0, 0.01, "Output[0] ≈ 1.0");

    free_conv_layer(&conv);
    return 0;
}

int test_conv_relu()
{
    printf("\n=== Test 3: ReLU Activation ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 1, 4, 4, 2, 3, 3, 1, 0);

    // Set first filter to produce negative values
    for (size_t i = 0; i < 9; i++) conv.kernels[i] = -0.1;
    conv.bias[0] = 0.0;

    // Set second filter to produce positive values
    for (size_t i = 9; i < 18; i++) conv.kernels[i] = 0.1;
    conv.bias[1] = 0.5;

    double input[16];
    for (int i = 0; i < 16; i++) input[i] = 1.0;

    forward_conv_layer(&conv, input);

    // Check that negative values are clipped to 0 (ReLU)
    for (size_t i = 0; i < 4; i++)  // First filter outputs (should be 0)
    {
        ASSERT(conv.output[i] >= 0.0, "ReLU: negative values clipped to 0");
    }

    // Check that positive values remain positive
    for (size_t i = 4; i < 8; i++)  // Second filter outputs
    {
        ASSERT(conv.output[i] > 0.0, "ReLU: positive values preserved");
    }

    free_conv_layer(&conv);
    return 0;
}

int test_conv_stride()
{
    printf("\n=== Test 4: Stride Effects ===\n");

    ConvLayer conv1, conv2;
    create_conv_layer(&conv1, 1, 8, 8, 1, 3, 3, 1, 0);  // stride=1
    create_conv_layer(&conv2, 1, 8, 8, 1, 3, 3, 2, 0);  // stride=2

    // stride=1: (8 - 3) / 1 + 1 = 6
    ASSERT(conv1.output_height == 6, "Stride 1: output height = 6");
    ASSERT(conv1.output_width == 6, "Stride 1: output width = 6");

    // stride=2: (8 - 3) / 2 + 1 = 3
    ASSERT(conv2.output_height == 3, "Stride 2: output height = 3");
    ASSERT(conv2.output_width == 3, "Stride 2: output width = 3");

    free_conv_layer(&conv1);
    free_conv_layer(&conv2);
    return 0;
}

int test_conv_padding()
{
    printf("\n=== Test 5: Padding Effects ===\n");

    ConvLayer conv1, conv2;
    create_conv_layer(&conv1, 1, 8, 8, 1, 3, 3, 1, 0);  // no padding
    create_conv_layer(&conv2, 1, 8, 8, 1, 3, 3, 1, 1);  // padding=1

    // no padding: (8 - 3) / 1 + 1 = 6
    ASSERT(conv1.output_height == 6, "No padding: output = 6");

    // padding=1: (8 + 2*1 - 3) / 1 + 1 = 8 (same size)
    ASSERT(conv2.output_height == 8, "Padding=1: output = 8 (same as input)");

    free_conv_layer(&conv1);
    free_conv_layer(&conv2);
    return 0;
}

int test_conv_multiple_channels()
{
    printf("\n=== Test 6: Multiple Input Channels ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 3, 8, 8, 16, 3, 3, 1, 1);

    ASSERT(conv.input_channels == 3, "Input channels = 3");
    ASSERT(conv.n_filters == 16, "Output channels = 16");

    // Kernel size: 16 filters * 3 channels * 3 * 3
    size_t expected_kernel_size = 16 * 3 * 3 * 3;
    size_t actual_kernel_size = conv.n_filters * conv.input_channels *
                                conv.kernel_height * conv.kernel_width;
    ASSERT(
        actual_kernel_size == expected_kernel_size,
        "Kernel size correct for multi-channel"
    );

    // Input size: 3 * 8 * 8 = 192
    double input[192];
    for (int i = 0; i < 192; i++) input[i] = 0.5;

    forward_conv_layer(&conv, input);

    // Output size: 16 * 8 * 8 = 1024
    size_t output_size =
        conv.n_filters * conv.output_height * conv.output_width;
    ASSERT(output_size == 1024, "Output size = 1024");

    free_conv_layer(&conv);
    return 0;
}

int test_conv_backward()
{
    printf("\n=== Test 7: Backward Pass ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 1, 4, 4, 2, 3, 3, 1, 0);

    // FORZAR kernels positivos para que ReLU no mate todo
    for (size_t i = 0; i < 18; i++)  // 2 filters * 9 weights
    {
        conv.kernels[i] = 0.1;
    }
    conv.bias[0] = 0.5;  // Bias positivo
    conv.bias[1] = 0.5;

    double input[16];
    for (int i = 0; i < 16; i++) input[i] = 1.0;  // Inputs positivos

    forward_conv_layer(&conv, input);

    printf("  Conv outputs after ReLU:\n");
    for (size_t i = 0; i < 4; i++)
    {
        printf("    filter 0, output[%zu] = %.6f\n", i, conv.output[i]);
    }
    for (size_t i = 4; i < 8; i++)
    {
        printf("    filter 1, output[%zu] = %.6f\n", i, conv.output[i]);
    }

    // Gradient from next layer (2 filters * 2x2 output = 8 values)
    double grad_output[8];
    for (int i = 0; i < 8; i++) grad_output[i] = 0.1;

    double *grad_kernels = NULL;
    double *grad_bias = NULL;
    get_empty_conv_gradients(&conv, &grad_kernels, &grad_bias);

    backward_conv_layer(&conv, grad_output, NULL, grad_kernels, grad_bias);

    // Debug prints
    printf("  Bias grad[0] = %.6f\n", grad_bias[0]);
    printf("  Bias grad[1] = %.6f\n", grad_bias[1]);
    printf("  Kernel grad[0] = %.6f\n", grad_kernels[0]);
    printf("  Kernel grad[1] = %.6f\n", grad_kernels[1]);

    int result = 0;

    // Check that gradients were computed
    int has_nonzero_grad = 0;
    for (size_t i = 0; i < 2; i++)
    {
        if (fabs(grad_bias[i]) > 1e-10)
        {
            has_nonzero_grad = 1;
            break;
        }
    }

    if (!has_nonzero_grad)
    {
        fprintf(
            stderr, "❌ FAIL: Bias gradients computed\n   at %s:%d\n", __FILE__,
            __LINE__
        );
        result = 1;
    }
    else
    {
        printf("✅ PASS: Bias gradients computed\n");
    }

    has_nonzero_grad = 0;
    for (size_t i = 0; i < 18; i++)
    {
        if (fabs(grad_kernels[i]) > 1e-10)
        {
            has_nonzero_grad = 1;
            break;
        }
    }

    if (!has_nonzero_grad)
    {
        fprintf(
            stderr, "❌ FAIL: Kernel gradients computed\n   at %s:%d\n",
            __FILE__, __LINE__
        );
        result = 1;
    }
    else
    {
        printf("✅ PASS: Kernel gradients computed\n");
    }

    free(grad_kernels);
    free(grad_bias);
    free_conv_layer(&conv);

    return result;
}

int test_conv_gradient_update()
{
    printf("\n=== Test 8: Parameter Update ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 1, 4, 4, 1, 3, 3, 1, 0);

    // FORZAR kernels positivos
    for (size_t i = 0; i < 9; i++) { conv.kernels[i] = 0.2; }
    conv.bias[0] = 0.5;

    double initial_kernel = conv.kernels[0];
    double initial_bias = conv.bias[0];

    double input[16];
    for (int i = 0; i < 16; i++) input[i] = 1.0;  // Positivos

    forward_conv_layer(&conv, input);

    printf("  Output[0] after forward: %.6f\n", conv.output[0]);

    double grad_output[4];
    for (int i = 0; i < 4; i++) grad_output[i] = 0.1;

    double *grad_kernels = NULL;
    double *grad_bias = NULL;
    get_empty_conv_gradients(&conv, &grad_kernels, &grad_bias);

    backward_conv_layer(&conv, grad_output, NULL, grad_kernels, grad_bias);

    double learning_rate = 0.01;
    update_conv_parameters(&conv, grad_kernels, grad_bias, learning_rate);

    printf(
        "  Kernel before: %.6f, after: %.6f (diff: %.6f)\n", initial_kernel,
        conv.kernels[0], conv.kernels[0] - initial_kernel
    );
    printf(
        "  Bias before: %.6f, after: %.6f (diff: %.6f)\n", initial_bias,
        conv.bias[0], conv.bias[0] - initial_bias
    );
    printf("  Kernel grad[0] = %.6f\n", grad_kernels[0]);
    printf("  Bias grad[0] = %.6f\n", grad_bias[0]);

    int result = 0;

    if (fabs(conv.kernels[0] - initial_kernel) <= 1e-10)
    {
        fprintf(
            stderr,
            "❌ FAIL: Kernels updated after gradient descent\n   at %s:%d\n",
            __FILE__, __LINE__
        );
        result = 1;
    }
    else
    {
        printf("✅ PASS: Kernels updated after gradient descent\n");
    }

    if (fabs(conv.bias[0] - initial_bias) <= 1e-10)
    {
        fprintf(
            stderr,
            "❌ FAIL: Bias updated after gradient descent\n   at %s:%d\n",
            __FILE__, __LINE__
        );
        result = 1;
    }
    else
    {
        printf("✅ PASS: Bias updated after gradient descent\n");
    }

    free(grad_kernels);
    free(grad_bias);
    free_conv_layer(&conv);

    return result;
}

int test_conv_col2im()
{
    printf("\n=== Test 9: col2im Inverse ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 1, 4, 4, 1, 3, 3, 1, 0);

    double input[16];
    for (int i = 0; i < 16; i++) input[i] = (double)i;

    // Do im2col
    im2col(input, 1, 4, 4, 3, 3, 1, 0, conv.col_buffer);

    // Do col2im back
    double output[16];
    col2im(conv.col_buffer, 1, 4, 4, 3, 3, 1, 0, output);

    // Note: col2im accumulates, and overlapping regions will have higher values
    // Just check that it doesn't crash and produces reasonable values
    ASSERT(output[5] > 0, "col2im produces output");

    printf("  col2im completed without crash\n");

    free_conv_layer(&conv);
    return 0;
}

int test_conv_known_values()
{
    printf("\n=== Test 10: Known Input/Output Values ===\n");

    ConvLayer conv;
    create_conv_layer(
        &conv, 1, 3, 3, 1, 2, 2, 1, 0
    );  // 3x3 input, 2x2 kernel, no padding

    // Set kernel to simple pattern
    conv.kernels[0] = 1.0;
    conv.kernels[1] = 0.0;
    conv.kernels[2] = 0.0;
    conv.kernels[3] = 1.0;
    conv.bias[0] = 0.0;

    // Input:
    // 1 2 3
    // 4 5 6
    // 7 8 9
    double input[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    forward_conv_layer(&conv, input);

    // Expected output (2x2):
    // Top-left: 1*1 + 2*0 + 4*0 + 5*1 = 6
    // Top-right: 2*1 + 3*0 + 5*0 + 6*1 = 8
    // Bottom-left: 4*1 + 5*0 + 7*0 + 8*1 = 12
    // Bottom-right: 5*1 + 6*0 + 8*0 + 9*1 = 14

    ASSERT(conv.output_height == 2, "Output height = 2");
    ASSERT(conv.output_width == 2, "Output width = 2");

    ASSERT_NEAR(conv.output[0], 6.0, 0.001, "Output[0,0] = 6.0");
    ASSERT_NEAR(conv.output[1], 8.0, 0.001, "Output[0,1] = 8.0");
    ASSERT_NEAR(conv.output[2], 12.0, 0.001, "Output[1,0] = 12.0");
    ASSERT_NEAR(conv.output[3], 14.0, 0.001, "Output[1,1] = 14.0");

    free_conv_layer(&conv);
    return 0;
}

int test_conv_gradient_values()
{
    printf("\n=== Test 11: Gradient Numerical Values ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 1, 3, 3, 1, 2, 2, 1, 0);

    // Simple kernel
    conv.kernels[0] = 0.5;
    conv.kernels[1] = 0.5;
    conv.kernels[2] = 0.5;
    conv.kernels[3] = 0.5;
    conv.bias[0] = 0.0;

    double input[9] = {1, 1, 1, 1, 1, 1, 1, 1, 1};  // All ones

    forward_conv_layer(&conv, input);

    // Each output = 0.5*1 + 0.5*1 + 0.5*1 + 0.5*1 = 2.0
    printf(
        "  Output values: %.3f, %.3f, %.3f, %.3f\n", conv.output[0],
        conv.output[1], conv.output[2], conv.output[3]
    );

    ASSERT_NEAR(conv.output[0], 2.0, 0.001, "Output = 2.0 for all ones input");

    // Gradient: uniform 1.0
    double grad_output[4] = {1.0, 1.0, 1.0, 1.0};

    double *grad_kernels = NULL;
    double *grad_bias = NULL;
    get_empty_conv_gradients(&conv, &grad_kernels, &grad_bias);

    backward_conv_layer(&conv, grad_output, NULL, grad_kernels, grad_bias);

    // Bias gradient = sum of all grad_output (ReLU is active since output > 0)
    // grad_bias = 1 + 1 + 1 + 1 = 4.0
    printf("  Bias gradient = %.6f (expected 4.0)\n", grad_bias[0]);
    ASSERT_NEAR(grad_bias[0], 4.0, 0.001, "Bias gradient = 4.0");

    // Kernel gradients should be sum of input patches weighted by grad_output
    printf(
        "  Kernel gradients: %.3f, %.3f, %.3f, %.3f\n", grad_kernels[0],
        grad_kernels[1], grad_kernels[2], grad_kernels[3]
    );

    // Each kernel position sees different number of 1s from overlapping patches
    ASSERT(grad_kernels[0] > 0, "Kernel gradient[0] computed");

    free(grad_kernels);
    free(grad_bias);
    free_conv_layer(&conv);
    return 0;
}

int test_conv_with_negative_inputs()
{
    printf("\n=== Test 12: Negative Inputs with ReLU ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 1, 4, 4, 1, 3, 3, 1, 0);

    // Kernel that produces mix of positive/negative
    for (size_t i = 0; i < 9; i++) conv.kernels[i] = 0.2;
    conv.bias[0] = -1.0;  // Negative bias

    double input[16];
    for (int i = 0; i < 16; i++) input[i] = 1.0;

    forward_conv_layer(&conv, input);

    // Output = sum(0.2 * 9 ones) - 1.0 = 1.8 - 1.0 = 0.8 (positive, survives
    // ReLU)
    printf("  Output[0] = %.6f (expected ~0.8)\n", conv.output[0]);
    ASSERT_NEAR(conv.output[0], 0.8, 0.01, "Output = 0.8 after ReLU");

    // Now test with larger negative bias
    conv.bias[0] = -5.0;
    forward_conv_layer(&conv, input);

    // Output = 1.8 - 5.0 = -3.2 -> ReLU -> 0.0
    printf("  Output[0] with bias=-5 = %.6f (expected 0.0)\n", conv.output[0]);
    ASSERT_NEAR(conv.output[0], 0.0, 0.001, "ReLU clips negative to 0");

    free_conv_layer(&conv);
    return 0;
}

int test_conv_learning_step()
{
    printf("\n=== Test 13: Full Learning Step ===\n");

    ConvLayer conv;
    create_conv_layer(
        &conv, 1, 5, 5, 1, 3, 3, 1, 1
    );  // 5x5 input, same size output

    // Initialize with small positive weights
    for (size_t i = 0; i < 9; i++) conv.kernels[i] = 0.1;
    conv.bias[0] = 0.1;

    double initial_kernel_sum = 0;
    for (size_t i = 0; i < 9; i++) initial_kernel_sum += conv.kernels[i];

    printf("  Initial kernel sum = %.6f\n", initial_kernel_sum);

    // Create simple pattern: vertical edge
    double input[25] = {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
                        0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0};

    // Do 10 learning steps
    for (int step = 0; step < 10; step++)
    {
        forward_conv_layer(&conv, input);

        // Fake gradient: want higher activation in center
        double grad_output[25];
        for (int i = 0; i < 25; i++) grad_output[i] = 0.0;
        grad_output[12] = 1.0;  // Center position only

        double *grad_kernels = NULL;
        double *grad_bias = NULL;
        get_empty_conv_gradients(&conv, &grad_kernels, &grad_bias);

        backward_conv_layer(&conv, grad_output, NULL, grad_kernels, grad_bias);
        update_conv_parameters(&conv, grad_kernels, grad_bias, 0.01);

        free(grad_kernels);
        free(grad_bias);
    }

    double final_kernel_sum = 0;
    for (size_t i = 0; i < 9; i++) final_kernel_sum += conv.kernels[i];

    printf("  Final kernel sum = %.6f\n", final_kernel_sum);
    printf(
        "  Kernel changed: %s\n",
        fabs(final_kernel_sum - initial_kernel_sum) > 0.001 ? "YES" : "NO"
    );

    ASSERT(
        fabs(final_kernel_sum - initial_kernel_sum) > 0.001,
        "Kernels updated after learning steps"
    );

    free_conv_layer(&conv);
    return 0;
}

int test_conv_bias_effect()
{
    printf("\n=== Test 14: Bias Effect ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 1, 4, 4, 1, 2, 2, 1, 0);

    // Zero kernel, non-zero bias
    for (size_t i = 0; i < 4; i++) conv.kernels[i] = 0.0;
    conv.bias[0] = 2.5;

    double input[16];
    for (int i = 0; i < 16; i++) input[i] = 1.0;

    forward_conv_layer(&conv, input);

    // Output should be just the bias (kernel is 0)
    printf("  Output[0] = %.6f (expected 2.5)\n", conv.output[0]);
    ASSERT_NEAR(
        conv.output[0], 2.5, 0.001, "Output equals bias when kernel is zero"
    );

    // All outputs should be the same
    for (size_t i = 1; i < 9; i++)
    {
        ASSERT_NEAR(conv.output[i], 2.5, 0.001, "All outputs equal bias");
    }

    free_conv_layer(&conv);
    return 0;
}

int test_conv_zero_input()
{
    printf("\n=== Test 15: Zero Input ===\n");

    ConvLayer conv;
    create_conv_layer(&conv, 1, 4, 4, 1, 3, 3, 1, 0);

    for (size_t i = 0; i < 9; i++) conv.kernels[i] = 0.5;
    conv.bias[0] = 0.0;

    double input[16] = {0};  // All zeros

    forward_conv_layer(&conv, input);

    // Output should be all zeros (0 * kernels + 0 bias = 0)
    ASSERT_NEAR(conv.output[0], 0.0, 0.001, "Zero input produces zero output");

    for (size_t i = 1; i < 4; i++)
    {
        ASSERT_NEAR(conv.output[i], 0.0, 0.001, "All outputs zero");
    }

    free_conv_layer(&conv);
    return 0;
}

// Actualiza el main para incluir los nuevos tests
int main()
{
    printf("╔══════════════════════════════════════╗\n");
    printf("║   ConvLayer Comprehensive Tests     ║\n");
    printf("╚══════════════════════════════════════╝\n");

    int failed = 0;

    failed += test_conv_creation();
    failed += test_conv_forward_basic();
    failed += test_conv_relu();
    failed += test_conv_stride();
    failed += test_conv_padding();
    failed += test_conv_multiple_channels();
    failed += test_conv_backward();
    failed += test_conv_gradient_update();
    failed += test_conv_col2im();
    failed += test_conv_known_values();          // NEW
    failed += test_conv_gradient_values();       // NEW
    failed += test_conv_with_negative_inputs();  // NEW
    failed += test_conv_learning_step();         // NEW
    failed += test_conv_bias_effect();           // NEW
    failed += test_conv_zero_input();            // NEW

    printf("\n");
    printf("═══════════════════════════════════════\n");
    if (failed == 0) { printf("✅ All %d tests PASSED!\n", 15); }
    else
    {
        printf("❌ %d test(s) FAILED!\n", failed);
    }
    printf("═══════════════════════════════════════\n");

    return failed;
}
