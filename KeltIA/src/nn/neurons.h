#pragma once

#include "stdlib.h"

struct Perceptron
{
    size_t num_entries;
    double *entries;
    double *biases;
};
