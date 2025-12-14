#include "../../include/nn/grid_builder.h"
#include "../../include/detect_zones/detect_zones.h"
#include "../../include/nn/network.h"
#include "../../include/nn/network_io.h"
#include <stdlib.h>
#include <string.h>

// Helper struct for sorting characters by spatial position
typedef struct
{
    int index;  // Original index in chars array
    int line;   // Assigned line number
    int x;      // X position
    int y;      // Y position
} CharIndex;

/**
 * Get predicted letter from CNN output (argmax)
 */
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

/**
 * Group characters into lines based on Y position
 * Characters within 10 pixels vertically are considered same line
 */
static int group_lines(CharBBox *chars, int num, int **assignments)
{
    if (num == 0) return 0;

    *assignments = malloc(num * sizeof(int));

    // First character starts first line
    int line = 0;
    (*assignments)[0] = 0;
    int line_y_sum[100] = {chars[0].y};  // Track Y sum per line (max 100 lines)
    int line_counts[100] = {1};          // Track char count per line

    for (int i = 1; i < num; i++)
    {
        int current_y = chars[i].y;
        int assigned = 0;

        // Try to assign to existing line
        for (int j = 0; j <= line; j++)
        {
            int avg_y = line_y_sum[j] / line_counts[j];
            if (abs(current_y - avg_y) <= 10)  // Threshold: 10 pixels
            {
                (*assignments)[i] = j;
                line_y_sum[j] += current_y;
                line_counts[j]++;
                assigned = 1;
                break;
            }
        }

        // Create new line if no match found
        if (!assigned)
        {
            line++;
            (*assignments)[i] = line;
            line_y_sum[line] = current_y;
            line_counts[line] = 1;
        }
    }

    return line + 1;  // Return total number of lines
}

/**
 * Comparison function for qsort - sorts by line then X position
 */
static int compare_char_position(const void *a, const void *b)
{
    const CharIndex *ca = (const CharIndex *)a;
    const CharIndex *cb = (const CharIndex *)b;

    // First sort by line (top to bottom)
    if (ca->line != cb->line) { return ca->line - cb->line; }

    // Then sort by X position (left to right)
    return ca->x - cb->x;
}

/**
 * Build complete grid result for UI
 */
GridResult build_grid_for_ui(CharBBox *chars, int num, char *predictions)
{
    GridResult result = {NULL, NULL, NULL, 0, 0};
    if (num == 0) return result;

    // Group characters into lines
    int *lines;
    int num_lines = group_lines(chars, num, &lines);

    // Create sorted index array
    CharIndex *char_indices = malloc(num * sizeof(CharIndex));
    for (int i = 0; i < num; i++)
    {
        char_indices[i].index = i;
        char_indices[i].line = lines[i];
        char_indices[i].x = chars[i].x;
        char_indices[i].y = chars[i].y;
    }

    // Sort by line first, then by X position (left to right)
    qsort(char_indices, num, sizeof(CharIndex), compare_char_position);

    // Count characters per line
    int *chars_per_line = calloc(num_lines, sizeof(int));
    for (int i = 0; i < num; i++) { chars_per_line[lines[i]]++; }

    // Find maximum number of columns
    int max_cols = 0;
    for (int i = 0; i < num_lines; i++)
    {
        if (chars_per_line[i] > max_cols) max_cols = chars_per_line[i];
    }

    result.height = num_lines;
    result.width = max_cols;

    // Allocate CharBBox grid (2D array)
    result.grid = malloc(num_lines * sizeof(CharBBox *));
    for (int i = 0; i < num_lines; i++)
    {
        result.grid[i] = calloc(max_cols, sizeof(CharBBox));
    }

    // Allocate char grid (flattened, null-terminated)
    result.char_grid = malloc((num_lines * max_cols) + 1);
    for (int i = 0; i < num_lines * max_cols; i++)
    {
        result.char_grid[i] = ' ';  // Initialize with spaces
    }
    result.char_grid[num_lines * max_cols] = '\0';

    // Fill both grids in sorted order
    int *col_count = calloc(num_lines, sizeof(int));
    for (int i = 0; i < num; i++)
    {
        int original_idx = char_indices[i].index;
        int line = char_indices[i].line;
        int col = col_count[line];

        if (col < max_cols)
        {
            // Copy CharBBox to grid
            result.grid[line][col] = chars[original_idx];

            // Set character in flattened array
            result.char_grid[line * max_cols + col] = predictions[original_idx];

            col_count[line]++;
        }
    }

    // Cleanup temporary allocations
    free(char_indices);
    free(lines);
    free(chars_per_line);
    free(col_count);

    return result;
}

/**
 * Build wordlist from characters
 */
char **build_wordlist_for_ui(
    CharBBox *chars,
    int num,
    char *predictions,
    int *num_words
)
{
    *num_words = 0;
    if (num == 0) return NULL;

    // Group characters into lines
    int *lines;
    int num_lines = group_lines(chars, num, &lines);

    // Sort characters by position
    CharIndex *char_indices = malloc(num * sizeof(CharIndex));
    for (int i = 0; i < num; i++)
    {
        char_indices[i].index = i;
        char_indices[i].line = lines[i];
        char_indices[i].x = chars[i].x;
        char_indices[i].y = chars[i].y;
    }

    qsort(char_indices, num, sizeof(CharIndex), compare_char_position);

    // Allocate word array (with extra slot for NULL terminator)
    char **words = malloc((num_lines + 1) * sizeof(char *));

    for (int line = 0; line < num_lines; line++)
    {
        int count = 0;

        // Count chars in this line
        for (int i = 0; i < num; i++)
        {
            if (char_indices[i].line == line) count++;
        }

        if (count == 0) continue;

        // Allocate word string (null-terminated)
        words[*num_words] = malloc(count + 1);
        int idx = 0;

        // Add chars in sorted order
        for (int i = 0; i < num; i++)
        {
            if (char_indices[i].line == line)
            {
                int original_idx = char_indices[i].index;
                words[*num_words][idx++] = predictions[original_idx];
            }
        }

        words[*num_words][idx] = '\0';  // Null-terminate word
        (*num_words)++;
    }

    // Null-terminate array
    words[*num_words] = NULL;

    free(char_indices);
    free(lines);

    return words;
}

/**
 * Classify characters using CNN model
 */
char *classify_characters(
    NeuronalNetwork *nn,
    MagickWand *wand,
    CharBBox *chars,
    int num
)
{
    if (num == 0) return NULL;

    char *predictions = malloc(num);

    for (int i = 0; i < num; i++)
    {
        double *input = charbbox_to_cnn_input(wand, chars[i]);
        double output[26];
        compute_nn(nn, input, output);
        double conf;
        predictions[i] = get_predicted_letter(output, &conf);
        free(input);
    }

    return predictions;
}

/**
 * COMPLETE PIPELINE: Process crossword image from file to result
 */
GridResult process_crossword_image(
    const char *image_path,
    const char *model_path,
    char ***out_words,
    int *out_num_words,
    int *error_code
)
{
    GridResult empty_result = {NULL, NULL, NULL, 0, 0};

    // Initialize output parameters
    if (out_words) *out_words = NULL;
    if (out_num_words) *out_num_words = 0;
    if (error_code) *error_code = CROSSWORD_OK;

    // Step 1: Initialize MagickWand
    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    if (!wand)
    {
        if (error_code) *error_code = CROSSWORD_ERR_MAGICK_INIT;
        MagickWandTerminus();
        return empty_result;
    }

    // Step 2: Load image
    if (!MagickReadImage(wand, image_path))
    {
        if (error_code) *error_code = CROSSWORD_ERR_IMAGE_LOAD;
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return empty_result;
    }

    // Step 3: Detect zones (grid and wordlist areas)
    ExtractedZones ez = detect_zones(wand);

    // Step 4: Detect individual characters in both zones
    int grid_count = 0, word_count = 0;

    CharBBox *grid_chars = detect_characters(
        wand, ez.grid.x_min, ez.grid.y_min, ez.grid.x_max - ez.grid.x_min,
        ez.grid.y_max - ez.grid.y_min, &grid_count
    );

    CharBBox *word_chars = detect_characters(
        wand, ez.words.x_min, ez.words.y_min, ez.words.x_max - ez.words.x_min,
        ez.words.y_max - ez.words.y_min, &word_count
    );

    // Check if we found characters
    if (grid_count == 0)
    {
        if (error_code) *error_code = CROSSWORD_ERR_NO_GRID;
        free(grid_chars);
        free(word_chars);
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return empty_result;
    }

    if (word_count == 0)
    {
        if (error_code) *error_code = CROSSWORD_ERR_NO_WORDS;
        free(grid_chars);
        free(word_chars);
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return empty_result;
    }

    // Step 5: Load CNN model
    NeuronalNetwork nn;
    if (load_nn(model_path, &nn) != NN_ERR_OK)
    {
        if (error_code) *error_code = CROSSWORD_ERR_MODEL_LOAD;
        free(grid_chars);
        free(word_chars);
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return empty_result;
    }

    // Step 6: Classify all characters using CNN
    char *grid_preds = classify_characters(&nn, wand, grid_chars, grid_count);
    char *word_preds = classify_characters(&nn, wand, word_chars, word_count);

    // Step 7: Build grid structure
    GridResult result = build_grid_for_ui(grid_chars, grid_count, grid_preds);

    // Step 8: Build wordlist
    int num_words = 0;
    char **words =
        build_wordlist_for_ui(word_chars, word_count, word_preds, &num_words);

    // Set output parameters
    if (out_words) *out_words = words;
    if (out_num_words) *out_num_words = num_words;

    // Cleanup ALL temporary resources (including wand)
    free(grid_chars);
    free(word_chars);
    free(grid_preds);
    free(word_preds);
    free_nn(&nn);
    DestroyMagickWand(wand);
    MagickWandTerminus();

    return result;
}

/**
 * ALTERNATIVE API: Process crossword and fill provided pointers
 */
int extract_crossword_data(
    const char *image_path,
    const char *model_path,
    CharBBox ***out_grid,
    char **out_char_grid,
    int *out_height,
    int *out_width,
    char ***out_words,
    int *out_num_words
)
{
    // Initialize all output pointers to NULL/0
    if (out_grid) *out_grid = NULL;
    if (out_char_grid) *out_char_grid = NULL;
    if (out_height) *out_height = 0;
    if (out_width) *out_width = 0;
    if (out_words) *out_words = NULL;
    if (out_num_words) *out_num_words = 0;

    // Step 1: Initialize MagickWand
    MagickWandGenesis();
    MagickWand *wand = NewMagickWand();

    if (!wand)
    {
        MagickWandTerminus();
        return CROSSWORD_ERR_MAGICK_INIT;
    }

    // Step 2: Load image
    if (!MagickReadImage(wand, image_path))
    {
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return CROSSWORD_ERR_IMAGE_LOAD;
    }

    // Step 3: Detect zones
    ExtractedZones ez = detect_zones(wand);

    // Step 4: Detect characters
    int grid_count = 0, word_count = 0;

    CharBBox *grid_chars = detect_characters(
        wand, ez.grid.x_min, ez.grid.y_min, ez.grid.x_max - ez.grid.x_min,
        ez.grid.y_max - ez.grid.y_min, &grid_count
    );

    CharBBox *word_chars = detect_characters(
        wand, ez.words.x_min, ez.words.y_min, ez.words.x_max - ez.words.x_min,
        ez.words.y_max - ez.words.y_min, &word_count
    );

    if (grid_count == 0)
    {
        free(grid_chars);
        free(word_chars);
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return CROSSWORD_ERR_NO_GRID;
    }

    if (word_count == 0)
    {
        free(grid_chars);
        free(word_chars);
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return CROSSWORD_ERR_NO_WORDS;
    }

    // Step 5: Load CNN model
    NeuronalNetwork nn;
    if (load_nn(model_path, &nn) != NN_ERR_OK)
    {
        free(grid_chars);
        free(word_chars);
        DestroyMagickWand(wand);
        MagickWandTerminus();
        return CROSSWORD_ERR_MODEL_LOAD;
    }

    // Step 6: Classify characters
    char *grid_preds = classify_characters(&nn, wand, grid_chars, grid_count);
    char *word_preds = classify_characters(&nn, wand, word_chars, word_count);

    // Step 7: Build grid
    GridResult result = build_grid_for_ui(grid_chars, grid_count, grid_preds);

    // Step 8: Build wordlist
    int num_words = 0;
    char **words =
        build_wordlist_for_ui(word_chars, word_count, word_preds, &num_words);

    // Step 9: Assign to output pointers
    if (out_grid) *out_grid = result.grid;
    if (out_char_grid) *out_char_grid = result.char_grid;
    if (out_height) *out_height = result.height;
    if (out_width) *out_width = result.width;
    if (out_words) *out_words = words;
    if (out_num_words) *out_num_words = num_words;

    // Cleanup temporary resources
    free(grid_chars);
    free(word_chars);
    free(grid_preds);
    free(word_preds);
    free_nn(&nn);
    DestroyMagickWand(wand);
    MagickWandTerminus();

    return CROSSWORD_OK;
}

/**
 * Free all resources from process_crossword_image
 */
void free_crossword_result(GridResult *result, char **words)
{
    // Free grid result
    if (result) { free_grid_result(result); }

    // Free wordlist
    if (words) { free_wordlist(words); }
}

/**
 * Free GridResult memory
 */
void free_grid_result(GridResult *result)
{
    if (result->grid)
    {
        for (int i = 0; i < result->height; i++) { free(result->grid[i]); }
        free(result->grid);
        result->grid = NULL;
    }

    if (result->char_grid)
    {
        free(result->char_grid);
        result->char_grid = NULL;
    }

    if (result->words)
    {
        free_wordlist(result->words);
        result->words = NULL;
    }

    result->height = 0;
    result->width = 0;
}

/**
 * Free wordlist array
 */
void free_wordlist(char **words)
{
    if (!words) return;

    for (int i = 0; words[i] != NULL; i++) { free(words[i]); }
    free(words);
}
