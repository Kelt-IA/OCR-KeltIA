#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int count_cols(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    char *line = NULL;
    size_t len = 0;
    int cols = 0;

    ssize_t read = getline(&line, &len, file);
    if (read != -1)
    {
        cols = read;
        if (cols > 0 && line[cols - 1] == '\n') { cols--; }
    }

    free(line);
    fclose(file);
    return cols;
}

int count_lines(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    int counter = 0;
    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, file) != -1) { counter++; }

    free(line);
    fclose(file);
    return counter;
}

char **readFile(const char *filename, int *rows, int *cols)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) return NULL;

    int r = count_lines(filename);
    int c = count_cols(filename);

    if (r <= 0 || c <= 0)
    {
        fclose(fp);
        return NULL;
    }

    char *grid = (char *)malloc(r * c * sizeof(char));
    if (!grid)
    {
        fclose(fp);
        return NULL;
    }

    char **result = (char **)malloc(r * sizeof(char *));
    if (!result)
    {
        free(grid);
        fclose(fp);
        return NULL;
    }

    for (int i = 0; i < r; i++) { result[i] = grid + (i * c); }

    char *line = NULL;
    size_t len = 0;
    int current_row = 0;

    while (getline(&line, &len, fp) != -1 && current_row < r)
    {
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len - 1] == '\n')
        {
            line[line_len - 1] = '\0';
            line_len--;
        }

        size_t chars_to_copy = (line_len < c) ? line_len : c;
        memcpy(result[current_row], line, chars_to_copy);

        if (chars_to_copy < c)
        {
            memset(result[current_row] + chars_to_copy, ' ', c - chars_to_copy);
        }

        current_row++;
    }

    free(line);
    fclose(fp);

    *rows = r;
    *cols = c;

    return result;
}

void free_grid(char **grid)
{
    if (grid)
    {
        free(grid[0]);
        free(grid);
    }
}
