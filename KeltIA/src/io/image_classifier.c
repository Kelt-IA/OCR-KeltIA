#include "../../include/io/image_classifier.h"
#include "../../include/io/bitmap_loader.h"
#include "../../include/nn/network_io.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

// Create output directory structure A-Z
int create_letter_folders(const char *output_dir)
{
    // Create main output directory
    mkdir(output_dir, 0755);

    // Create subfolder for each letter A-Z
    for (char letter = 'A'; letter <= 'Z'; letter++)
    {
        char folder_path[512];
        snprintf(folder_path, sizeof(folder_path), "%s/%c", output_dir, letter);

        if (mkdir(folder_path, 0755) != 0 && errno != EEXIST)
        {
            fprintf(
                stderr, "Error creating folder %s: %s\n", folder_path,
                strerror(errno)
            );
            return 0;
        }
    }

    printf("✓ Created letter folders A-Z in %s\n", output_dir);
    return 1;
}

// Get predicted class from network output
static int
get_predicted_class(double *output, size_t output_size, double *out_confidence)
{
    int max_idx = 0;
    double max_val = output[0];

    for (size_t i = 1; i < output_size; i++)
    {
        if (output[i] > max_val)
        {
            max_val = output[i];
            max_idx = i;
        }
    }

    *out_confidence = max_val;
    return max_idx;
}

// Predict single image
ImagePrediction *predict_single_image(
    NeuronalNetwork *nn,
    const char *filepath,
    size_t width,
    size_t height
)
{
    ImagePrediction *pred = malloc(sizeof(ImagePrediction));
    if (!pred)
    {
        fprintf(stderr, "Error allocating ImagePrediction\n");
        return NULL;
    }

    // Load image using MagickWand
    size_t img_width, img_height;
    uint8_t *pixels =
        load_single_image_magick(filepath, &img_width, &img_height);

    if (!pixels)
    {
        free(pred);
        return NULL;
    }

    // Verify dimensions
    if (img_width != width || img_height != height)
    {
        fprintf(
            stderr, "Warning: image %s has size %zux%zu, expected %zux%zu\n",
            filepath, img_width, img_height, width, height
        );
        free(pixels);
        free(pred);
        return NULL;
    }

    // Normalize pixels to [0, 1]
    size_t input_size = width * height;
    double *input = malloc(input_size * sizeof(double));
    if (!input)
    {
        fprintf(stderr, "Error allocating input buffer\n");
        free(pixels);
        free(pred);
        return NULL;
    }

    for (size_t i = 0; i < input_size; i++) { input[i] = pixels[i] / 255.0; }

    // Allocate output buffer
    size_t output_size = nn->layers[nn->n_layers - 1].n_neurons;
    double *output = malloc(output_size * sizeof(double));
    if (!output)
    {
        fprintf(stderr, "Error allocating output buffer\n");
        free(input);
        free(pixels);
        free(pred);
        return NULL;
    }

    // Forward pass through network
    compute_nn(nn, input, output);

    // Get prediction
    double confidence;
    int predicted_class = get_predicted_class(output, output_size, &confidence);

    // Fill prediction structure
    pred->filepath = strdup(filepath);
    pred->pixels = pixels;
    pred->width = img_width;
    pred->height = img_height;
    pred->predicted_class = predicted_class;
    pred->confidence = confidence;

    free(input);
    free(output);

    return pred;
}

// Free prediction
void free_prediction(ImagePrediction *pred)
{
    if (!pred) return;

    if (pred->filepath) free(pred->filepath);
    if (pred->pixels) free(pred->pixels);
    free(pred);
}

// Copy file to destination
static int copy_file(const char *src, const char *dst)
{
    FILE *src_f = fopen(src, "rb");
    if (!src_f)
    {
        fprintf(stderr, "Error opening source file: %s\n", src);
        return 0;
    }

    FILE *dst_f = fopen(dst, "wb");
    if (!dst_f)
    {
        fprintf(stderr, "Error opening destination file: %s\n", dst);
        fclose(src_f);
        return 0;
    }

    // Copy in chunks
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_f)) > 0)
    {
        if (fwrite(buffer, 1, bytes, dst_f) != bytes)
        {
            fprintf(stderr, "Error writing to destination file\n");
            fclose(src_f);
            fclose(dst_f);
            return 0;
        }
    }

    fclose(src_f);
    fclose(dst_f);
    return 1;
}

// Check if file is an image (by extension)
int is_image_file(const char *filename)
{
    size_t len = strlen(filename);
    if (len < 4) return 0;

    const char *ext = filename + len - 4;
    return (
        strcasecmp(ext, ".bmp") == 0 || strcasecmp(ext, ".png") == 0 ||
        strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext + 1, ".jpeg") == 0
    );
}

// Main classification and organization function
int lassify_and_organize_images(
    NeuronalNetwork *nn,
    const char *input_dir,
    const char *output_dir,
    size_t image_width,
    size_t image_height
)
{
    // Create output folder structure
    if (!create_letter_folders(output_dir)) { return 0; }

    // Open input directory
    DIR *dir = opendir(input_dir);
    if (!dir)
    {
        fprintf(stderr, "Error opening input directory: %s\n", input_dir);
        return 0;
    }

    printf("\nClassifying images from %s...\n", input_dir);

    struct dirent *entry;
    size_t total_processed = 0;
    size_t total_success = 0;
    size_t class_counts[26] = {0};  // Count per letter

    // Process each file
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip directories and non-image files
        if (entry->d_type == DT_DIR || !is_image_file(entry->d_name))
        {
            continue;
        }

        // Build input file path
        char input_path[512];
        snprintf(
            input_path, sizeof(input_path), "%s/%s", input_dir, entry->d_name
        );

        // Predict
        ImagePrediction *pred =
            predict_single_image(nn, input_path, image_width, image_height);

        if (!pred)
        {
            fprintf(stderr, "Warning: failed to process %s\n", entry->d_name);
            continue;
        }

        total_processed++;

        // Convert class index to letter (0=A, 1=B, ..., 25=Z)
        char predicted_letter = 'A' + pred->predicted_class;

        // Build output file path
        char output_path[512];
        snprintf(
            output_path, sizeof(output_path), "%s/%c/%s", output_dir,
            predicted_letter, entry->d_name
        );

        // Copy file to predicted folder
        if (copy_file(input_path, output_path))
        {
            total_success++;
            class_counts[pred->predicted_class]++;

            printf(
                "[%zu] %s -> %c (confidence: %.2f%%)\n", total_processed,
                entry->d_name, predicted_letter, pred->confidence * 100
            );
        }
        else
        {
            fprintf(
                stderr, "Error copying %s to %s\n", entry->d_name, output_path
            );
        }

        free_prediction(pred);
    }

    closedir(dir);

    // Print summary
    printf("\n========== CLASSIFICATION SUMMARY ==========\n");
    printf("Total images processed: %zu\n", total_processed);
    printf("Successfully classified: %zu\n", total_success);
    printf("\nDistribution by letter:\n");

    for (int i = 0; i < 26; i++)
    {
        if (class_counts[i] > 0)
        {
            printf("  %c: %zu images\n", 'A' + i, class_counts[i]);
        }
    }
    printf("==========================================\n");

    return 1;
}
