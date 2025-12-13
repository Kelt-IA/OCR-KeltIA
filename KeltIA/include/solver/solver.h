#pragma once
#include <stdlib.h>

typedef struct
{
    int x1, y1, x2, y2;
} WordPos;

WordPos *
solver(char *grid, int height, int width, const char *word, int word_len);
