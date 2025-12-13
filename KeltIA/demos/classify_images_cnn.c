// demos/classify_images_cnn.c
#include "../include/io/bitmap_loader.h"
#include "../include/nn/network.h"
#include "../include/nn/network_io.h"
#include <MagickWand/MagickWand.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#define CONFIDENCE_THRESHOLD 0.50  // 50% threshold

// Create output directory structure A-Z + low_confidence folder
static int create_letter_folders(const char *output_dir)
{
    mkdir(output_dir, 0755);

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

    // Create low_confidence folder
    char low_conf_path[512];
    snprintf(
        low_conf_path, sizeof(low_conf_path), "%s/low_confidence", output_dir
    );
    if (mkdir(low_conf_path, 0755) != 0 && errno != EEXIST)
    {
        fprintf(
            stderr, "Error creating low_confidence folder: %s\n",
            strerror(errno)
        );
        return 0;
    }

    printf("✓ Created letter folders A-Z + low_confidence in %s\n", output_dir);
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

// Check if file is an image
static int is_image_file(const char *filename)
{
    size_t len = strlen(filename);
    if (len < 4) return 0;

    const char *ext = filename + len - 4;
    return (
        strcasecmp(ext, ".bmp") == 0 || strcasecmp(ext, ".png") == 0 ||
        strcasecmp(ext, ".jpg") == 0 || strcasecmp(ext + 1, ".jpeg") == 0
    );
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <model_path> <images_folder>\n", argv[0]);
        fprintf(
            stderr,
            "Example: %s models/mnist/mnist-cnn-final.nn ./extracted_letters\n",
            argv[0]
        );
        fprintf(
            stderr, "\nClassified images will be saved to: ./classified/\n"
        );
        fprintf(
            stderr,
            "Low confidence images (< %.0f%%) will be saved to: "
            "./classified/low_confidence/\n",
            CONFIDENCE_THRESHOLD * 100
        );
        return EXIT_FAILURE;
    }

    const char *model_path = argv[1];
    const char *input_dir = argv[2];
    const char *output_dir = "./classified";

    printf("╔════════════════════════════════════════╗\n");
    printf("║   Image Classification Organizer     ║\n");
    printf("╚════════════════════════════════════════╝\n\n");

    // Load trained CNN model
    printf("Loading model: %s\n", model_path);
    NeuronalNetwork nn;
    ErrorCode err = load_nn(model_path, &nn);

    if (err != NN_ERR_OK)
    {
        fprintf(
            stderr, "✗ Failed to load model: %s\n", nn_error_to_string(err)
        );
        return EXIT_FAILURE;
    }

    printf("✓ Model loaded successfully\n");
    printf("  Conv layers: %zu\n", nn.n_conv_layers);
    printf("  Dense layers: %zu\n", nn.n_layers);

    // Get output classes
    size_t output_classes = nn.layers[nn.n_layers - 1].n_neurons;
    printf("  Output classes: %zu ", output_classes);

    if (output_classes == 26) { printf("(A-Z letters)\n"); }
    else if (output_classes == 10) { printf("(0-9 digits)\n"); }
    else
    {
        printf("(custom)\n");
    }

    printf("\nInput directory: %s\n", input_dir);
    printf("Output directory: %s\n", output_dir);
    printf("Confidence threshold: %.0f%%\n", CONFIDENCE_THRESHOLD * 100);

    // Initialize MagickWand
    MagickWandGenesis();

    // Create output folder structure
    if (!create_letter_folders(output_dir))
    {
        MagickWandTerminus();
        free_nn(&nn);
        return EXIT_FAILURE;
    }

    // Open input directory
    DIR *dir = opendir(input_dir);
    if (!dir)
    {
        fprintf(stderr, "Error opening input directory: %s\n", input_dir);
        MagickWandTerminus();
        free_nn(&nn);
        return EXIT_FAILURE;
    }

    printf("\nClassifying images from %s...\n\n", input_dir);

    struct dirent *entry;
    size_t total_processed = 0;
    size_t total_success = 0;
    size_t low_confidence_count = 0;
    size_t class_counts[26] = {0};

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

        // Load and resize image to 28x28
        uint8_t *pixels = resize_and_pad_to_28x28(input_path);
        if (!pixels)
        {
            fprintf(stderr, "Warning: failed to process %s\n", entry->d_name);
            continue;
        }

        // Normalize pixels to [0, 1]
        size_t input_size = 28 * 28;
        double *input = malloc(input_size * sizeof(double));
        if (!input)
        {
            free(pixels);
            continue;
        }

        for (size_t i = 0; i < input_size; i++)
        {
            input[i] = pixels[i] / 255.0;
        }

        // Allocate output buffer
        double *output = malloc(output_classes * sizeof(double));
        if (!output)
        {
            free(input);
            free(pixels);
            continue;
        }

        // Forward pass through network
        compute_nn(&nn, input, output);

        // Get prediction
        double confidence;
        int predicted_class =
            get_predicted_class(output, output_classes, &confidence);

        total_processed++;

        // Convert class index to letter (0=A, 1=B, ..., 25=Z)
        char predicted_letter = 'A' + predicted_class;

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
            class_counts[predicted_class]++;

            // Check if confidence is low
            if (confidence < CONFIDENCE_THRESHOLD)
            {
                // Also save to low_confidence folder with renamed file
                char low_conf_filename[768];
                snprintf(
                    low_conf_filename, sizeof(low_conf_filename),
                    "%s/low_confidence/%c_%.2f_%s", output_dir,
                    predicted_letter, confidence * 100, entry->d_name
                );

                if (copy_file(input_path, low_conf_filename))
                {
                    low_confidence_count++;
                    printf(
                        "[%zu] %s -> %c (confidence: %.2f%%) ⚠️ LOW\n",
                        total_processed, entry->d_name, predicted_letter,
                        confidence * 100
                    );
                }
            }
            else
            {
                printf(
                    "[%zu] %s -> %c (confidence: %.2f%%)\n", total_processed,
                    entry->d_name, predicted_letter, confidence * 100
                );
            }
        }
        else
        {
            fprintf(
                stderr, "Error copying %s to %s\n", entry->d_name, output_path
            );
        }

        // Cleanup
        free(output);
        free(input);
        free(pixels);
    }

    closedir(dir);

    // Print summary
    printf("\n========== CLASSIFICATION SUMMARY ==========\n");
    printf("Total images processed: %zu\n", total_processed);
    printf("Successfully classified: %zu\n", total_success);
    printf(
        "Low confidence images: %zu (%.1f%%)\n", low_confidence_count,
        (low_confidence_count * 100.0) / total_processed
    );
    printf("\nDistribution by letter:\n");

    for (int i = 0; i < 26; i++)
    {
        if (class_counts[i] > 0)
        {
            printf("  %c: %zu images\n", 'A' + i, class_counts[i]);
        }
    }
    printf("==========================================\n");

    // Cleanup
    MagickWandTerminus();
    free_nn(&nn);

    printf("\n╔════════════════════════════════════════╗\n");
    printf("║   Classification Complete!            ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\nCheck your organized images in: %s/\n", output_dir);
    printf("Review low confidence images in: %s/low_confidence/\n", output_dir);

    return EXIT_SUCCESS;
}
