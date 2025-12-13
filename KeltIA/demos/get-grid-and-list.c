// demos/solve_crossword.c - Complete OCR crossword solver
#include "../include/detect_zones/detect_char.h"
#include "../include/detect_zones/detect_zones.h"
#include "../include/image/grayscale.h"
#include "../include/image/removenoise.h"
#include "../include/nn/network.h"
#include "../include/nn/network_io.h"
#include <MagickWand/MagickWand.h>
#include <err.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_MODE 0  // Set to 1 to save debug images

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

// Get predicted letter from CNN
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

// Extract and normalize character to 28x28
static double *
extract_char(MagickWand *wand, CharBBox *bbox, int zone_x, int zone_y)
{
    MagickWand *char_wand = CloneMagickWand(wand);

    int abs_x = zone_x + bbox->x;
    int abs_y = zone_y + bbox->y;

    MagickCropImage(char_wand, bbox->w, bbox->h, abs_x, abs_y);
    MagickResetImagePage(char_wand, "");
    MagickSetImageColorspace(char_wand, GRAYColorspace);

    // Resize to 22x22
    size_t orig_w = MagickGetImageWidth(char_wand);
    size_t orig_h = MagickGetImageHeight(char_wand);

    double scale = 22.0 / (orig_w > orig_h ? orig_w : orig_h);
    if (scale > 1.0) scale = 1.0;

    size_t new_w = (size_t)(orig_w * scale);
    size_t new_h = (size_t)(orig_h * scale);

    MagickResizeImage(char_wand, new_w, new_h, LanczosFilter);

    // Create 28x28 white canvas
    MagickWand *canvas = NewMagickWand();
    PixelWand *white = NewPixelWand();
    PixelSetColor(white, "white");
    MagickNewImage(canvas, 28, 28, white);
    DestroyPixelWand(white);

    int x_off = (28 - new_w) / 2;
    int y_off = (28 - new_h) / 2;
    MagickCompositeImage(
        canvas, char_wand, OverCompositeOp, MagickTrue, x_off, y_off
    );

    // Export and normalize
    uint8_t *pixels = malloc(28 * 28);
    MagickExportImagePixels(canvas, 0, 0, 28, 28, "I", CharPixel, pixels);

    double *normalized = malloc(28 * 28 * sizeof(double));
    for (int i = 0; i < 28 * 28; i++) { normalized[i] = pixels[i] / 255.0; }

    free(pixels);
    DestroyMagickWand(char_wand);
    DestroyMagickWand(canvas);

    return normalized;
}

// Group chars into lines
static int group_lines(CharBBox *chars, int num, int **assignments)
{
    if (num == 0) return 0;

    *assignments = malloc(num * sizeof(int));
    int line = 0;
    (*assignments)[0] = 0;
    int last_y = chars[0].y;

    for (int i = 1; i < num; i++)
    {
        if (abs(chars[i].y - last_y) > 10)
        {
            line++;
            last_y = chars[i].y;
        }
        (*assignments)[i] = line;
    }

    return line + 1;
}

// Build grid
static Grid build_grid(CharBBox *chars, int num, char *predictions)
{
    Grid grid = {NULL, 0, 0};
    if (num == 0) return grid;

    int *lines;
    int num_lines = group_lines(chars, num, &lines);

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
    for (int i = 0; i < num; i++)
    {
        int line = lines[i];
        int col = col_count[line]++;
        if (col < max_cols) { grid.grid[line][col] = predictions[i]; }
    }

    free(lines);
    free(chars_per_line);
    free(col_count);

    return grid;
}

// Build wordlist
static Word *
build_wordlist(CharBBox *chars, int num, char *predictions, int *num_words)
{
    *num_words = 0;
    if (num == 0) return NULL;

    int *lines;
    int num_lines = group_lines(chars, num, &lines);

    Word *words = malloc(num_lines * sizeof(Word));

    for (int line = 0; line < num_lines; line++)
    {
        int count = 0;
        for (int i = 0; i < num; i++)
        {
            if (lines[i] == line) count++;
        }

        if (count == 0) continue;

        words[*num_words].text = malloc(count + 1);
        words[*num_words].length = count;

        int idx = 0;
        for (int i = 0; i < num; i++)
        {
            if (lines[i] == line)
            {
                words[*num_words].text[idx++] = predictions[i];
            }
        }
        words[*num_words].text[idx] = '\0';
        (*num_words)++;
    }

    free(lines);
    return words;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input_image> <cnn_model>\n", argv[0]);
        fprintf(
            stderr, "Example: %s crossword.png models/custom-cnn-final.nn\n",
            argv[0]
        );
        return EXIT_FAILURE;
    }

    const char *input_path = argv[1];
    const char *model_path = argv[2];

    printf("╔════════════════════════════════════════╗\n");
    printf("║     Crossword OCR Solver with CNN    ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    MagickWandGenesis();

    // Step 1: Correct image
    printf("[1/4] Correcting image...\n");
    MagickWand *wand_temp = binarize_image_wand(input_path);
    if (!wand_temp)
    {
        MagickWandTerminus();
        return EXIT_FAILURE;
    }

    MagickWand *wand = remove_noise(wand_temp);
    DestroyMagickWand(wand_temp);
    if (!wand)
    {
        MagickWandTerminus();
        return EXIT_FAILURE;
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

    // Classify grid
    char *grid_preds = malloc(grid_count);
    for (int i = 0; i < grid_count; i++)
    {
        double *input =
            extract_char(wand, &grid_chars[i], ez.grid.x_min, ez.grid.y_min);
        double output[26];
        compute_nn(&nn, input, output);

        double conf;
        grid_preds[i] = get_predicted_letter(output, &conf);
        free(input);
    }

    // Classify words
    char *word_preds = malloc(word_count);
    for (int i = 0; i < word_count; i++)
    {
        double *input =
            extract_char(wand, &word_chars[i], ez.words.x_min, ez.words.y_min);
        double output[26];
        compute_nn(&nn, input, output);

        double conf;
        word_preds[i] = get_predicted_letter(output, &conf);
        free(input);
    }

    printf("✓ Done\n\n");

    // Step 4: Build and display
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
