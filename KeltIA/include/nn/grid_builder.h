#ifndef GRID_BUILDER_H
#define GRID_BUILDER_H

#include "../detect_zones/detect_char.h"
#include "network.h"
#include <MagickWand/MagickWand.h>

/**
 * Result structure containing grid and wordlist data for UI
 */
typedef struct
{
    CharBBox **grid;  // 2D array: grid[row][col]
    char *char_grid;  // Flattened char array (null-terminated)
    char **words;     // Array of word strings (null-terminated)
    int height;       // Number of rows in grid
    int width;        // Number of columns in grid
} GridResult;

/**
 * Error codes for process_crossword_image
 */
typedef enum
{
    CROSSWORD_OK = 0,
    CROSSWORD_ERR_IMAGE_LOAD = 1,
    CROSSWORD_ERR_MODEL_LOAD = 2,
    CROSSWORD_ERR_NO_GRID = 3,
    CROSSWORD_ERR_NO_WORDS = 4,
    CROSSWORD_ERR_MAGICK_INIT = 5
} CrosswordError;

/**
 * Build complete grid result from detected characters and CNN predictions
 *
 * @param chars Array of detected character bounding boxes
 * @param num Number of characters detected
 * @param predictions Array of predicted characters from CNN
 * @return GridResult structure with all grid data (heap allocated)
 *
 * Note: Caller must free result using free_grid_result()
 */
GridResult build_grid_for_ui(CharBBox *chars, int num, char *predictions);

/**
 * Build wordlist from detected characters and CNN predictions
 *
 * @param chars Array of detected character bounding boxes
 * @param num Number of characters detected
 * @param predictions Array of predicted characters from CNN
 * @param num_words Output parameter: number of words extracted
 * @return Null-terminated array of null-terminated word strings (heap
 * allocated)
 *
 * Note: Caller must free each word and the array itself
 */
char **build_wordlist_for_ui(
    CharBBox *chars,
    int num,
    char *predictions,
    int *num_words
);

/**
 * Classify characters using CNN model
 *
 * @param nn Loaded neural network
 * @param wand MagickWand containing the image
 * @param chars Array of character bounding boxes to classify
 * @param num Number of characters
 * @return Heap-allocated array of predicted characters (caller must free)
 */
char *classify_characters(
    NeuronalNetwork *nn,
    MagickWand *wand,
    CharBBox *chars,
    int num
);

/**
 * COMPLETE PIPELINE: Process crossword image from file path to final result
 *
 * This is the main function your UI should call. It does everything:
 * - Initializes MagickWand
 * - Loads the image
 * - Detects grid and wordlist zones
 * - Detects individual characters
 * - Loads CNN model
 * - Classifies all characters
 * - Builds grid and wordlist structures
 * - Cleans up internal resources (MagickWand, model, etc.)
 *
 * @param image_path Path to crossword image file
 * @param model_path Path to CNN model file (.nn)
 * @param out_words Output: pointer to receive wordlist array (null-terminated)
 * @param out_num_words Output: pointer to receive number of words
 * @param error_code Output: pointer to receive error code (can be NULL)
 * @return GridResult with grid data. On error, returns empty GridResult and
 * sets error_code
 *
 * IMPORTANT: Caller must cleanup with:
 * - free_crossword_result() to free grid and words
 * OR manually:
 * - free_grid_result() for grid
 * - free_wordlist() for words
 *
 * Example usage:
 *   char **words;
 *   int num_words;
 *   int error;
 *   GridResult grid = process_crossword_image(
 *       "puzzle.png", "model.nn", &words, &num_words, &error
 *   );
 *   if (error == CROSSWORD_OK) {
 *       show_result(grid.grid, grid.char_grid, grid.height, grid.width, words);
 *       free_crossword_result(&grid, words);
 *   }
 */
GridResult process_crossword_image(
    const char *image_path,
    const char *model_path,
    char ***out_words,
    int *out_num_words,
    int *error_code
);

/**
 * ALTERNATIVE API: Process crossword and fill provided pointers
 *
 * This function gives you direct control over all output parameters.
 * All outputs are heap-allocated and must be freed by caller.
 *
 * @param image_path Path to crossword image file
 * @param model_path Path to CNN model file (.nn)
 * @param out_grid Output: 2D array of CharBBox (grid[row][col])
 * @param out_char_grid Output: flattened char array (null-terminated)
 * @param out_height Output: number of rows in grid
 * @param out_width Output: number of columns in grid
 * @param out_words Output: null-terminated array of word strings
 * @param out_num_words Output: number of words
 * @return Error code (CROSSWORD_OK on success)
 *
 * Example usage:
 *   CharBBox **grid;
 *   char *char_grid;
 *   int height, width;
 *   char **words;
 *   int num_words;
 *
 *   int error = extract_crossword_data(
 *       "puzzle.png", "model.nn",
 *       &grid, &char_grid, &height, &width,
 *       &words, &num_words
 *   );
 *
 *   if (error == CROSSWORD_OK) {
 *       // Use data however you want
 *       show_result(grid, char_grid, height, width, words);
 *
 *       // Manual cleanup
 *       for (int i = 0; i < height; i++) free(grid[i]);
 *       free(grid);
 *       free(char_grid);
 *       for (int i = 0; words[i] != NULL; i++) free(words[i]);
 *       free(words);
 *   }
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
);

/**
 * Free all resources from process_crossword_image
 *
 * @param result Pointer to GridResult to free
 * @param words Wordlist array to free
 */
void free_crossword_result(GridResult *result, char **words);

/**
 * Free GridResult memory
 *
 * @param result Pointer to GridResult to free
 */
void free_grid_result(GridResult *result);

/**
 * Free wordlist array
 *
 * @param words Null-terminated array of word strings
 */
void free_wordlist(char **words);

#endif  // GRID_BUILDER_H
