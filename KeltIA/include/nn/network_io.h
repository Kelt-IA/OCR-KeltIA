#pragma once

#include "network.h"

// used for verifying the format of the file
#define MAGIC "NNET"
#define MAGIC_SIZE 4

typedef enum
{
    ERR_OK = 0,
    ERR_FILE_OPEN = -1,
    ERR_FORMAT = -2,
    ERR_MEMORY = -3,
    ERR_READ = -4,
    ERR_WRITE = -5
} ErrorCode;

int save_nn(char *path, const NeuronalNetwork *nn);
int load_nn(const char *path, NeuronalNetwork *out_nn);
