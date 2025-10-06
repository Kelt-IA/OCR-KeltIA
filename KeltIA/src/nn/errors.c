#include "../../include/nn/include_nn.h"
#include <stdio.h>

void print_nn_error(ErrorCode err, const char *context)
{
    if (err == NN_ERR_OK) return;

    fprintf(stderr, "[ERROR] ");

    if (context) { fprintf(stderr, "%s: ", context); }

    switch (err)
    {
    case NN_ERR_FILE_OPEN:
        fprintf(stderr, "Failed to open file\n");
        break;
    case NN_ERR_FORMAT:
        fprintf(stderr, "Invalid file format or corrupted data\n");
        break;
    case NN_ERR_MEMORY:
        fprintf(stderr, "Memory allocation failed\n");
        break;
    case NN_ERR_READ:
        fprintf(stderr, "Failed to read from file\n");
        break;
    case NN_ERR_WRITE:
        fprintf(stderr, "Failed to write to file\n");
        break;
    case NN_ERR_NULL_POINTER:
        fprintf(stderr, "Null pointer encountered\n");
        break;
    default:
        fprintf(stderr, "Unknown error (code: %d)\n", err);
        break;
    }
}

// Versión que devuelve el string del error (útil para tests)
const char *nn_error_to_string(ErrorCode err)
{
    switch (err)
    {
    case NN_ERR_OK:
        return "OK";
    case NN_ERR_FILE_OPEN:
        return "File open error";
    case NN_ERR_FORMAT:
        return "Format error";
    case NN_ERR_MEMORY:
        return "Memory error";
    case NN_ERR_READ:
        return "Read error";
    case NN_ERR_WRITE:
        return "Write error";
    case NN_ERR_NULL_POINTER:
        return "Null pointer error";
    default:
        return "Unknown error";
    }
}
