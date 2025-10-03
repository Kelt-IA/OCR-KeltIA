#pragma once

typedef enum
{
    NN_ERR_OK = 0,
    NN_ERR_FILE_OPEN = -1,
    NN_ERR_FORMAT = -2,
    NN_ERR_MEMORY = -3,
    NN_ERR_READ = -4,
    NN_ERR_WRITE = -5,
    NN_ERR_NULL_POINTER = -10,
} ErrorCode;
