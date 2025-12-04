#include "../../include/nn/include_nn.h"
#include <stdio.h>

const char *TEST_FILE = "ressources/xor.nn";

// ANSI color codes
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

void xor_nn_perceptron(NeuronalNetwork *nn)
{
    // XOR 2->2->1
    size_t num_neuron_l[2] = {2, 1};

    create_nn(2, 2, num_neuron_l, nn);

    // force step activation
    for (size_t i = 0; i < nn->n_layers; i++)
    {
        set_activation(&nn->layers[i], ACTIVATION_STEP);
    }

    // hand crafted weights and biases
    double first_w[4] = {1, 1, 1, 1};  // 2 neurons, 2 inputs each -> 4 weights
    double first_b[2] = {-0.5, -1.5};  // 2 neurons, 2 biases

    double second_w[2] = {1, -1};  // 1 neuron,  2 inputs -> 2 weights
    double second_b[1] = {-0.5};   // 1 neuron,  1 bias

    // layer 1 (hidden layer)
    load_weights(&nn->layers[0], first_w);
    load_biases(&nn->layers[0], first_b);

    // layer 2 (exit)
    load_weights(&nn->layers[1], second_w);
    load_biases(&nn->layers[1], second_b);
}

void print_double_array_compact(const double *arr, size_t size)
{
    printf("[");
    for (size_t i = 0; i < size; i++)
    {
        printf("%.2f", arr[i]);
        if (i < size - 1) printf(", ");
    }
    printf("]");
}

void print_weights_matrix(
    const double *weights,
    size_t n_neurons,
    size_t n_inputs
)
{
    printf("  [\n");
    for (size_t neuron = 0; neuron < n_neurons; neuron++)
    {
        printf("    [");
        for (size_t input = 0; input < n_inputs; input++)
        {
            size_t idx = neuron * n_inputs + input;
            printf("%.2f", weights[idx]);
            if (input < n_inputs - 1) printf(", ");
        }
        printf("]");
        if (neuron < n_neurons - 1) printf(",");
        printf("\n");
    }
    printf("  ]");
}

int compare_layers(
    const Layer *a,
    const Layer *b,
    int verbose,
    size_t layer_idx
)
{
    int result = 1;

    if (verbose)
    {
        printf(COLOR_BLUE "\n--- Layer %zu ---\n" COLOR_RESET, layer_idx);
        printf("Config: %zu inputs, %zu neurons\n", a->n_inputs, a->n_neurons);
    }

    if (a->n_inputs != b->n_inputs)
    {
        if (verbose)
        {
            printf(
                COLOR_RED "✗ n_inputs: %zu vs %zu\n" COLOR_RESET, a->n_inputs,
                b->n_inputs
            );
        }
        return 0;
    }

    if (a->n_neurons != b->n_neurons)
    {
        if (verbose)
        {
            printf(
                COLOR_RED "✗ n_neurons: %zu vs %zu\n" COLOR_RESET, a->n_neurons,
                b->n_neurons
            );
        }
        return 0;
    }

    // Compare weights
    size_t weight_count = a->n_neurons * a->n_inputs;
    int weights_match = 1;
    for (size_t i = 0; i < weight_count; i++)
    {
        if (a->weights[i] != b->weights[i])
        {
            weights_match = 0;
            break;
        }
    }

    if (verbose)
    {
        printf("\nWeights [%zu×%zu]:\n", a->n_neurons, a->n_inputs);
        printf("A:\n");
        print_weights_matrix(a->weights, a->n_neurons, a->n_inputs);
        printf("\nB:\n");
        print_weights_matrix(b->weights, b->n_neurons, b->n_inputs);
        printf("\n");

        if (weights_match) { printf(COLOR_GREEN "  ✓ MATCH\n" COLOR_RESET); }
        else
        {
            printf(COLOR_RED "  ✗ DIFFERENT\n" COLOR_RESET);
            result = 0;
        }
    }
    else if (!weights_match) { return 0; }

    // Compare biases
    int biases_match = 1;
    for (size_t i = 0; i < a->n_neurons; i++)
    {
        if (a->bias[i] != b->bias[i])
        {
            biases_match = 0;
            break;
        }
    }

    if (verbose)
    {
        printf("\nBiases:\n");
        printf("  A: ");
        print_double_array_compact(a->bias, a->n_neurons);
        printf("\n");
        printf("  B: ");
        print_double_array_compact(b->bias, a->n_neurons);
        printf("\n");

        if (biases_match) { printf(COLOR_GREEN "  ✓ MATCH\n" COLOR_RESET); }
        else
        {
            printf(COLOR_RED "  ✗ DIFFERENT\n" COLOR_RESET);
            result = 0;
        }
    }
    else if (!biases_match) { return 0; }

    return result;
}

int compare_nn(const NeuronalNetwork *a, const NeuronalNetwork *b, int verbose)
{
    if (verbose)
    {
        printf(COLOR_BLUE "\n ========================================\n");
        printf("      Neural Network Comparison\n");
        printf("========================================\n" COLOR_RESET);
        printf("Layers: %zu\n", a->n_layers);
    }

    if (a->n_layers != b->n_layers)
    {
        if (verbose)
        {
            printf(
                COLOR_RED "✗ n_layers: %zu vs %zu\n" COLOR_RESET, a->n_layers,
                b->n_layers
            );
        }
        return 0;
    }

    int all_match = 1;
    for (size_t i = 0; i < a->n_layers; i++)
    {
        if (!compare_layers(&a->layers[i], &b->layers[i], verbose, i))
        {
            all_match = 0;
            if (!verbose) { return 0; }
        }
    }

    if (verbose)
    {
        printf(
            COLOR_BLUE
            "\n========================================\n" COLOR_RESET
        );
        if (all_match)
        {
            printf(COLOR_GREEN "       ✓ NETWORKS IDENTICAL\n" COLOR_RESET);
        }
        else
        {
            printf(COLOR_RED "       ✗ NETWORKS DIFFERENT\n" COLOR_RESET);
        }
        printf(
            COLOR_BLUE
            "========================================\n\n" COLOR_RESET
        );
    }

    return all_match;
}

const char *TEST_FILE_CNN = "ressources/simple_cnn.nn";

int compare_conv_layers(
    const ConvLayer *a,
    const ConvLayer *b,
    int verbose,
    size_t layer_idx
)
{
    int result = 1;

    if (verbose)
    {
        printf(COLOR_BLUE "\n--- Conv Layer %zu ---\n" COLOR_RESET, layer_idx);
        printf(
            "Config: %zux%zux%zu input, %zu filters, kernel %zux%zu, stride "
            "%zu, padding %zu\n",
            a->input_height, a->input_width, a->input_channels, a->n_filters,
            a->kernel_height, a->kernel_width, a->stride, a->padding
        );
    }

    // Compare configuration
    if (a->input_channels != b->input_channels ||
        a->input_height != b->input_height ||
        a->input_width != b->input_width || a->n_filters != b->n_filters ||
        a->kernel_height != b->kernel_height ||
        a->kernel_width != b->kernel_width || a->stride != b->stride ||
        a->padding != b->padding)
    {
        if (verbose)
        {
            printf(COLOR_RED "✗ Configuration mismatch\n" COLOR_RESET);
        }
        return 0;
    }

    // Compare kernels
    size_t kernel_size =
        a->n_filters * a->input_channels * a->kernel_height * a->kernel_width;
    int kernels_match = 1;

    for (size_t i = 0; i < kernel_size; i++)
    {
        if (a->kernels[i] != b->kernels[i])
        {
            kernels_match = 0;
            break;
        }
    }

    if (verbose)
    {
        printf("\nKernels [%zu total weights]:\n", kernel_size);
        if (kernels_match) { printf(COLOR_GREEN "  ✓ MATCH\n" COLOR_RESET); }
        else
        {
            printf(COLOR_RED "  ✗ DIFFERENT\n" COLOR_RESET);
            result = 0;
        }
    }
    else if (!kernels_match) { return 0; }

    // Compare biases
    int biases_match = 1;
    for (size_t i = 0; i < a->n_filters; i++)
    {
        if (a->bias[i] != b->bias[i])
        {
            biases_match = 0;
            break;
        }
    }

    if (verbose)
    {
        printf("\nBiases:\n");
        printf("  A: ");
        print_double_array_compact(a->bias, a->n_filters);
        printf("\n");
        printf("  B: ");
        print_double_array_compact(b->bias, a->n_filters);
        printf("\n");

        if (biases_match) { printf(COLOR_GREEN "  ✓ MATCH\n" COLOR_RESET); }
        else
        {
            printf(COLOR_RED "  ✗ DIFFERENT\n" COLOR_RESET);
            result = 0;
        }
    }
    else if (!biases_match) { return 0; }

    return result;
}

int compare_cnn(const NeuronalNetwork *a, const NeuronalNetwork *b, int verbose)
{
    if (verbose)
    {
        printf(COLOR_BLUE "\n========================================\n");
        printf("           CNN Comparison\n");
        printf("========================================\n" COLOR_RESET);
        printf(
            "Conv layers: %zu, Dense layers: %zu\n", a->n_conv_layers,
            a->n_layers
        );
    }

    // Compare structure
    if (a->n_conv_layers != b->n_conv_layers)
    {
        if (verbose)
        {
            printf(
                COLOR_RED "✗ n_conv_layers: %zu vs %zu\n" COLOR_RESET,
                a->n_conv_layers, b->n_conv_layers
            );
        }
        return 0;
    }

    if (a->n_layers != b->n_layers)
    {
        if (verbose)
        {
            printf(
                COLOR_RED "✗ n_layers: %zu vs %zu\n" COLOR_RESET, a->n_layers,
                b->n_layers
            );
        }
        return 0;
    }

    int all_match = 1;

    // Compare conv layers
    for (size_t i = 0; i < a->n_conv_layers; i++)
    {
        if (!compare_conv_layers(
                &a->conv_layers[i], &b->conv_layers[i], verbose, i
            ))
        {
            all_match = 0;
            if (!verbose) return 0;
        }
    }

    // Compare dense layers
    for (size_t i = 0; i < a->n_layers; i++)
    {
        if (!compare_layers(&a->layers[i], &b->layers[i], verbose, i))
        {
            all_match = 0;
            if (!verbose) return 0;
        }
    }

    if (verbose)
    {
        printf(
            COLOR_BLUE
            "\n========================================\n" COLOR_RESET
        );
        if (all_match)
        {
            printf(COLOR_GREEN "       ✓ CNNs IDENTICAL\n" COLOR_RESET);
        }
        else
        {
            printf(COLOR_RED "       ✗ CNNs DIFFERENT\n" COLOR_RESET);
        }
        printf(
            COLOR_BLUE
            "========================================\n\n" COLOR_RESET
        );
    }

    return all_match;
}

int test_cnn_save_load(int verbose)
{
    ConvLayer conv_configs[1];
    create_conv_layer(&conv_configs[0], 1, 8, 8, 2, 3, 3, 1, 0);

    // Set kernel values
    for (size_t i = 0; i < 2 * 1 * 3 * 3; i++)
    {
        conv_configs[0].kernels[i] = i * 0.1;
    }

    conv_configs[0].bias[0] = 0.98009;  // Magic numbersssss
    conv_configs[0].bias[1] = -0.73671;

    // size_t dense_neurons[] = {3};
    // ActivationType activations[] = {ACTIVATION_SIGMOID};

    // create_cnn(1, conv_configs, 1, dense_neurons, activations, &cnn);

    NeuronalNetwork cnn;

    size_t dense_neurons[] = {128, 10};  // Asegúrate de que esté definido
    ActivationType activations[] = {ACTIVATION_LEAKY_RELU, ACTIVATION_SIGMOID};

    create_cnn(
        1,              // n_conv_layers
        conv_configs,   // conv configs
        2,              // n_dense_layers
        dense_neurons,  // dense neurons array
        activations,    // activations array
        &cnn            // output network
    );

    // Randomize ALL dense layer parameters
    for (size_t l = 0; l < cnn.n_layers; l++)
    {
        // Randomize weights
        size_t weight_size = cnn.layers[l].n_neurons * cnn.layers[l].n_inputs;
        for (size_t i = 0; i < weight_size; i++)
        {
            cnn.layers[l].weights[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        }

        // Randomize biases (IMPORTANTE - no dejar en 0)
        for (size_t i = 0; i < cnn.layers[l].n_neurons; i++)
        {
            cnn.layers[l].bias[i] = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        }
    }

    ErrorCode err = save_nn(TEST_FILE_CNN, &cnn);
    if (err != NN_ERR_OK)
    {
        free_nn(&cnn);
        return 0;
    }

    NeuronalNetwork loaded_cnn;
    err = load_nn(TEST_FILE_CNN, &loaded_cnn);
    if (err != NN_ERR_OK)
    {
        free_nn(&cnn);
        return 0;
    }

    int equal = compare_cnn(&cnn, &loaded_cnn, verbose);

    free_nn(&cnn);
    free_nn(&loaded_cnn);

    return equal;
}

int test_write_nn()
{
    NeuronalNetwork nn;
    xor_nn_perceptron(&nn);

    ErrorCode err = save_nn(TEST_FILE, &nn);

    free_nn(&nn);
    if (err != 0)
    {
        fprintf(stderr, "%s\n", nn_error_to_string(err));
        return 0;
    }
    return 1;
}

int test_load_nn()
{
    NeuronalNetwork nn;
    memset(&nn, 0, sizeof(NeuronalNetwork));

    ErrorCode err = load_nn(TEST_FILE, &nn);

    free_nn(&nn);
    if (err != 0)
    {
        printf("%s\n", nn_error_to_string(err));
        return 0;
    }
    return 1;
}

int test_write_and_load(int verbose)
{
    NeuronalNetwork a;
    xor_nn_perceptron(&a);
    save_nn(TEST_FILE, &a);

    NeuronalNetwork b;
    memset(&b, 0, sizeof(NeuronalNetwork));
    load_nn(TEST_FILE, &b);

    int equal = compare_nn(&a, &b, verbose);

    free_nn(&a);
    free_nn(&b);

    return equal;
}
