#pragma once

#include "errors.h"
#include "network.h"

// used for verifying the format of the file
#define MAGIC "NNET"
#define MAGIC_SIZE 4

ErrorCode save_nn(const char *path, const NeuronalNetwork *nn);
ErrorCode load_nn(const char *path, NeuronalNetwork *out_nn);

// structure of custom file:
// (binary format)
// Line breaks (\n) added here only for better readability.

/*
MAGIC
{n_conv_layers}

// For each conv layer:
{input_channels}
{input_height}
{input_width}
{n_filters}
{kernel_height}
{kernel_width}
{stride}
{padding}
{kernels}
{biases}

{n_dense_layers}

// For each dense layer:
{n_neurons_layer_i}
{activation_type}
{biases_layer_i}
{weights_layer_i}
*/
