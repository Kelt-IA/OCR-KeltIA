#pragma once

#include <stdlib.h>

typedef struct
{
    char **labels;
    char ***rows;
    size_t num_columns;
    size_t entries;
} CSV;

CSV *read_csv(const char *filename, const char *sep);

void free_csv(CSV *data);
