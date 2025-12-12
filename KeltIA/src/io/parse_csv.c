#include "../../include/io/parse_csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Arbitrary values, magic numbers ✨
#define MAX_LINE_LENGTH 4096
#define MAX_COLUMNS 256
#define MAX_ENTRIES 10000

char **parse_line(const char *line, size_t *num_fields, const char *sep)
{
    char *line_copy = strdup(line);
    char **fields = malloc(MAX_COLUMNS * sizeof(char *));
    *num_fields = 0;

    char *token = strtok(line_copy, sep);
    while (token && *num_fields < MAX_COLUMNS)
    {
        char *trimmed = malloc(strlen(token) + 1);
        sscanf(token, "%s", trimmed);
        fields[(*num_fields)++] = trimmed;
        token = strtok(NULL, sep);
    }
    free(line_copy);
    return fields;  // Devuelves el array, pero esto NO se libera en read_csv
}

CSV *read_csv(const char *filename, const char *sep)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) return NULL;

    CSV *data = malloc(sizeof(CSV));
    char line[MAX_LINE_LENGTH];

    // Parse header
    if (fgets(line, sizeof(line), fp))
    {
        line[strcspn(line, "\n")] = 0;
        data->labels = parse_line(line, &data->num_columns, sep);
    }

    // Parse data
    data->rows = malloc(data->num_columns * sizeof(char **));
    for (size_t i = 0; i < data->num_columns; i++)
    {
        data->rows[i] = malloc(MAX_ENTRIES * sizeof(char *));
    }

    data->entries = 0;
    while (fgets(line, sizeof(line), fp) && data->entries < MAX_ENTRIES)
    {
        line[strcspn(line, "\n")] = 0;
        size_t num_fields = 0;
        char **fields = parse_line(line, &num_fields, sep);

        for (size_t i = 0; i < num_fields; i++)
        {
            data->rows[i][data->entries] = fields[i];
        }
        free(
            fields
        );  // LIBERA EL ARRAY, pero NO los strings (que ya están en data->rows)
        data->entries++;
    }

    fclose(fp);
    return data;
}

void free_csv(CSV *data)
{
    for (size_t i = 0; i < data->num_columns; i++)
    {
        free(data->labels[i]);
        for (size_t j = 0; j < data->entries; j++)
        {
            free(data->rows[i][j]);  // Esto está bien
        }
        free(data->rows[i]);
    }
    free(data->labels);
    free(data->rows);
    free(data);
}
