// demos/construct_grid_and_word_list.c - extract chars, classify with CNN,
// and build textual grid and word list

#include "../include/detect_zones/detect_char.h"
#include "../include/detect_zones/detect_zones.h"
#include "../include/image/grayscale.h"
#include "../include/image/removenoise.h"
#include "../include/io/bitmap_loader.h"
#include "../include/nn/network.h"
#include "../include/nn/network_io.h"
#include <MagickWand/MagickWand.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *text;
    int length;
} Word;

typedef struct
{
    char **grid;
    int rows;
    int cols;
} Grid;

// Helper struct for sorting characters
typedef struct
{
    int index;  // Original index in chars array
    int line;   // Assigned line number
    int x;      // X position
    int y;      // Y position
} CharIndex;

// Get predicted letter from CNN output (argmax)
static char get_predicted_letter(double *output, double *confidence)
{
    int max_idx = 0;
    double max_val = output[0];

    for (int i = 1; i < 26; i++)
    {
        if (output[i] > max_val)
        {
            max_val = output[i];
            max_idx = i;
        }
    }

    *confidence = max_val;
    return 'A' + max_idx;
}

// Group chars into lines by Y position
static int group_lines(CharBBox *chars, int num, int **assignments)
{
    if (num == 0) return 0;

    *assignments = malloc(num * sizeof(int));

    // First pass: assign preliminary lines
    int line = 0;
    (*assignments)[0] = 0;
    int line_y_sum[100] = {chars[0].y};  // Assuming max 100 lines
    int line_counts[100] = {1};

    for (int i = 1; i < num; i++)
    {
        int current_y = chars[i].y;
        int assigned = 0;

        // Try to assign to existing line
        for (int j = 0; j <= line; j++)
        {
            int avg_y = line_y_sum[j] / line_counts[j];
            if (abs(current_y - avg_y) <= 10)
            {
                (*assignments)[i] = j;
                line_y_sum[j] += current_y;
                line_counts[j]++;
                assigned = 1;
                break;
            }
        }

        // Create new line if no match
        if (!assigned)
        {
            line++;
            (*assignments)[i] = line;
            line_y_sum[line] = current_y;
            line_counts[line] = 1;
        }
    }

    return line + 1;
}

// Comparison function for sorting characters by position
static int compare_char_position(const void *a, const void *b)
{
    const CharIndex *ca = (const CharIndex *)a;
    const CharIndex *cb = (const CharIndex *)b;

    // First sort by Y position (top to bottom) - using average line Y
    if (ca->line != cb->line) { return ca->line - cb->line; }

    // Then sort by X position (left to right)
    return ca->x - cb->x;
}

// Build grid from chars and predictions - FIXED VERSION
static Grid build_grid(CharBBox *chars, int num, char *predictions)
{
    Grid grid = {NULL, 0, 0};
    if (num == 0) return grid;

    int *lines;
    int num_lines = group_lines(chars, num, &lines);

    // ===== FIX: Create sorted index array =====
    CharIndex *char_indices = malloc(num * sizeof(CharIndex));
    for (int i = 0; i < num; i++)
    {
        char_indices[i].index = i;
        char_indices[i].line = lines[i];
        char_indices[i].x = chars[i].x;
        char_indices[i].y = chars[i].y;
    }

    // Sort by line first, then by X position
    qsort(char_indices, num, sizeof(CharIndex), compare_char_position);
    // ===== END FIX =====

    int *chars_per_line = calloc(num_lines, sizeof(int));
    for (int i = 0; i < num; i++) { chars_per_line[lines[i]]++; }

    int max_cols = 0;
    for (int i = 0; i < num_lines; i++)
    {
        if (chars_per_line[i] > max_cols) max_cols = chars_per_line[i];
    }

    grid.rows = num_lines;
    grid.cols = max_cols;
    grid.grid = malloc(num_lines * sizeof(char *));

    for (int i = 0; i < num_lines; i++)
    {
        grid.grid[i] = malloc(max_cols + 1);
        memset(grid.grid[i], ' ', max_cols);
        grid.grid[i][max_cols] = '\0';
    }

    int *col_count = calloc(num_lines, sizeof(int));

    // ===== FIX: Process in sorted order =====
    for (int i = 0; i < num; i++)
    {
        int original_idx = char_indices[i].index;
        int line = char_indices[i].line;
        int col = col_count[line]++;

        if (col < max_cols)
        {
            grid.grid[line][col] = predictions[original_idx];
        }
    }
    // ===== END FIX =====

    free(char_indices);
    free(lines);
    free(chars_per_line);
    free(col_count);

    return grid;
}

// Build wordlist: group by lines and concatenate - FIXED VERSION
static Word *
build_wordlist(CharBBox *chars, int num, char *predictions, int *num_words)
{
    *num_words = 0;
    if (num == 0) return NULL;

    int *lines;
    int num_lines = group_lines(chars, num, &lines);

    // ===== FIX: Sort characters by position =====
    CharIndex *char_indices = malloc(num * sizeof(CharIndex));
    for (int i = 0; i < num; i++)
    {
        char_indices[i].index = i;
        char_indices[i].line = lines[i];
        char_indices[i].x = chars[i].x;
        char_indices[i].y = chars[i].y;
    }

    qsort(char_indices, num, sizeof(CharIndex), compare_char_position);
    // ===== END FIX =====

    Word *words = malloc(num_lines * sizeof(Word));

    for (int line = 0; line < num_lines; line++)
    {
        int count = 0;

        // Count chars in this line
        for (int i = 0; i < num; i++)
        {
            if (char_indices[i].line == line) count++;
        }

        if (count == 0) continue;

        words[*num_words].text = malloc(count + 1);
        words[*num_words].length = count;
        int idx = 0;

        // ===== FIX: Add chars in sorted order =====
        for (int i = 0; i < num; i++)
        {
            if (char_indices[i].line == line)
            {
                int original_idx = char_indices[i].index;
                words[*num_words].text[idx++] = predictions[original_idx];
            }
        }
        // ===== END FIX =====

        words[*num_words].text[idx] = '\0';
        (*num_words)++;
    }

    free(char_indices);
    free(lines);

    return words;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <image> <model>\n", argv[0]);
        fprintf(
            stderr, "Example: %s crossword.png models/custom-cnn-final.nn\n",
            argv[0]
        );
        return EXIT_FAILURE;
    }

    const char *input_path = argv[1];
    const char *model_path = argv[2];

    printf("╔════════════════════════════════════════╗\n");
    printf("║  Construct Grid and Word List         ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    MagickWandGenesis();

    // Step 1: Load image (assumed already preprocessed: grayscale + denoised)
    printf("[1/4] Loading image (assumed preprocessed)...\n");
    MagickWand *wand = NewMagickWand();

    if (!MagickReadImage(wand, input_path))
    {
        DestroyMagickWand(wand);
        MagickWandTerminus();
        errx(EXIT_FAILURE, "Error: Could not read %s\n", input_path);
    }

    printf("✓ Done\n\n");

    // Step 2: Detect zones and characters
    printf("[2/4] Detecting zones and characters...\n");
    ExtractedZones ez = detect_zones(wand);

    int grid_count = 0, word_count = 0;
    CharBBox *grid_chars = detect_characters(
        wand, ez.grid.x_min, ez.grid.y_min, ez.grid.x_max - ez.grid.x_min,
        ez.grid.y_max - ez.grid.y_min, &grid_count
    );

    CharBBox *word_chars = detect_characters(
        wand, ez.words.x_min, ez.words.y_min, ez.words.x_max - ez.words.x_min,
        ez.words.y_max - ez.words.y_min, &word_count
    );

    printf("  Grid: %d chars, Wordlist: %d chars\n", grid_count, word_count);
    printf("✓ Done\n\n");

    // Step 3: Load CNN and classify
    printf("[3/4] Loading CNN and classifying...\n");
    NeuronalNetwork nn;

    if (load_nn(model_path, &nn) != NN_ERR_OK)
    {
        fprintf(stderr, "Error loading model\n");
        free(grid_chars);
        free(word_chars);
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return EXIT_FAILURE;
    }

    // Classify grid chars
    char *grid_preds = malloc(grid_count ? grid_count : 1);
    for (int i = 0; i < grid_count; i++)
    {
        double *input = charbbox_to_cnn_input(wand, grid_chars[i]);
        double output[26];
        compute_nn(&nn, input, output);
        double conf;
        grid_preds[i] = get_predicted_letter(output, &conf);
        free(input);
    }

    // Classify wordlist chars
    char *word_preds = malloc(word_count ? word_count : 1);
    for (int i = 0; i < word_count; i++)
    {
        double *input = charbbox_to_cnn_input(wand, word_chars[i]);
        double output[26];
        compute_nn(&nn, input, output);
        double conf;
        word_preds[i] = get_predicted_letter(output, &conf);
        free(input);
    }

    printf("✓ Done\n\n");

    // Step 4: Build and print textual grid and wordlist
    printf("[4/4] Building grid and wordlist...\n\n");
    Grid grid = build_grid(grid_chars, grid_count, grid_preds);

    printf("╔════════════════════════════════════════╗\n");
    printf("║              GRID                     ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    for (int i = 0; i < grid.rows; i++) { printf("  %s\n", grid.grid[i]); }

    printf("\n");

    int num_words;
    Word *wordlist =
        build_wordlist(word_chars, word_count, word_preds, &num_words);

    printf("╔════════════════════════════════════════╗\n");
    printf("║            WORD LIST                  ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    for (int i = 0; i < num_words; i++)
    {
        printf("  %2d. %s\n", i + 1, wordlist[i].text);
    }

    printf("\n");

    // Cleanup
    for (int i = 0; i < grid.rows; i++) free(grid.grid[i]);
    free(grid.grid);

    for (int i = 0; i < num_words; i++) free(wordlist[i].text);
    free(wordlist);

    free(grid_preds);
    free(word_preds);
    free(grid_chars);
    free(word_chars);
    free_nn(&nn);

    DestroyMagickWand(wand);
    MagickWandTerminus();

    printf("✓ Done!\n");
    return EXIT_SUCCESS;
}
