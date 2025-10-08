#include "../include/nn/include_nn.h"
#include <stdio.h>

const char *TEST_FILE = "tests/xor.nn";

// ANSI color codes
#define COLOR_GREEN "\033[0;32m"
#define COLOR_RED "\033[0;31m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_RESET "\033[0m"

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
        printf("Inputs: %zu | Layers: %zu\n", a->n_inputs, a->n_layers);
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
        else { printf(COLOR_RED "       ✗ NETWORKS DIFFERENT\n" COLOR_RESET); }
        printf(
            COLOR_BLUE
            "========================================\n\n" COLOR_RESET
        );
    }

    return all_match;
}

/* Utilities functions */
// int compare_layers(const Layer *a, const Layer *b)
// {
//     if (a->n_inputs != b->n_inputs) return 0;
//     if (a->n_neurons != b->n_neurons) return 0;
//
//     for (size_t i = 0; i < a->n_neurons * a->n_inputs; i++)
//     {
//         if (a->weights[i] != b->weights[i]) return 0;
//     }
//
//     for (size_t i = 0; i < a->n_neurons; i++)
//     {
//         if (a->bias[i] != b->bias[i]) return 0;
//         if (a->output[i] != b->output[i]) return 0;
//     }
//
//     return 1;
// }
//
// int compare_nn(const NeuronalNetwork *a, const NeuronalNetwork *b)
// {
//     if (a->n_inputs != b->n_inputs) return 0;
//     if (a->n_layers != b->n_layers) return 0;
//
//     for (size_t i = 0; i < a->n_layers; i++)
//     {
//         if (!compare_layers(&a->layers[i], &b->layers[i])) return 0;
//     }
//     return 1;
// }

int test_write_nn()
{
    NeuronalNetwork nn;
    xor_nn(&nn);

    ErrorCode err = save_nn(TEST_FILE, &nn);

    free_nn(&nn);
    if (err != 0)
    {
        printf("%s\n", nn_error_to_string(err));
        // fprintf(stderr, "%s\n", nn_error_to_string(err));
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
        // fprintf(stderr, "%s\n", nn_error_to_string(err));
        return 0;
    }
    return 1;
}

int test_write_and_load(int verbose)
{
    NeuronalNetwork a;
    xor_nn(&a);
    save_nn(TEST_FILE, &a);

    NeuronalNetwork b;
    memset(&b, 0, sizeof(NeuronalNetwork));
    load_nn(TEST_FILE, &b);

    int equal = compare_nn(&a, &b, verbose);

    free_nn(&a);
    free_nn(&b);

    return equal;
}
