#pragma once

#include "network.h"

// used for verifying the format of the file
#define MAGIC "NNET"
#define MAGIC_SIZE 4

int save_nn(const char *path, const NeuronalNetwork *nn);
int load_nn(const char *path, NeuronalNetwork *out_nn);

// structure of custom file:
// (binary format)
// Line breaks (\n) added here only for better readability.

/*
MAGIC
{n_inputs}
{n_layers}

{n_neurons_layer_1}
{biases_layer_1}
{weights_layer_1}

{n_neurons_layer_2}
{biases_layer_2}
{weights_layer_2}

...

{n_neurons_layer_n}
{biases_layer_n}
{weights_layer_n}
*/
