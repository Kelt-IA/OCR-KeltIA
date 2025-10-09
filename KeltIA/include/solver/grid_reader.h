#pragma once

char **readFile(const char *filename, int *rows, int *cols);
void free_grid(char **grid);
