#include "../../include/io/bitmap_loader.h"
#include "../../include/nn/include_nn.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Load single image using MagickWand and convert to grayscale
uint8_t *load_single_image_magick(
    const char *filepath,
    size_t *out_width,
    size_t *out_height
)
{
    MagickWand *wand = NewMagickWand();
    MagickBooleanType status;

    // Read image
    status = MagickReadImage(wand, filepath);
    if (status == MagickFalse)
    {
        char *description;
        ExceptionType severity;
        description = MagickGetException(wand, &severity);
        fprintf(stderr, "Error loading image %s: %s\n", filepath, description);
        MagickRelinquishMemory(description);
        DestroyMagickWand(wand);
        return NULL;
    }

    // Convert to grayscale
    MagickSetImageColorspace(wand, GRAYColorspace);
    MagickSetImageType(wand, GrayscaleType);

    // Get dimensions
    size_t width = MagickGetImageWidth(wand);
    size_t height = MagickGetImageHeight(wand);

    *out_width = width;
    *out_height = height;

    // Export pixels as grayscale (8-bit)
    size_t image_size = width * height;
    uint8_t *pixels = malloc(image_size * sizeof(uint8_t));

    if (!pixels)
    {
        fprintf(stderr, "Error allocating pixel buffer\n");
        DestroyMagickWand(wand);
        return NULL;
    }

    // Export pixel data (I = Intensity/Grayscale, 0-255)
    status = MagickExportImagePixels(
        wand, 0, 0, width, height, "I", CharPixel, pixels
    );

    if (status == MagickFalse)
    {
        fprintf(stderr, "Error exporting pixels from %s\n", filepath);
        free(pixels);
        DestroyMagickWand(wand);
        return NULL;
    }

    DestroyMagickWand(wand);
    return pixels;
}

// Resize and pad image to exactly 28x28 maintaining aspect ratio
// with padding around the letter
uint8_t *resize_and_pad_to_28x28(const char *filepath)
{
    MagickWand *wand = NewMagickWand();
    MagickBooleanType status;

    // Read image
    status = MagickReadImage(wand, filepath);
    if (status == MagickFalse)
    {
        char *description;
        ExceptionType severity;
        description = MagickGetException(wand, &severity);
        fprintf(stderr, "Error loading image %s: %s\n", filepath, description);
        MagickRelinquishMemory(description);
        DestroyMagickWand(wand);
        return NULL;
    }

    // STEP 1: Convert to grayscale FIRST
    MagickSetImageColorspace(wand, GRAYColorspace);
    MagickSetImageType(wand, GrayscaleType);

    // Get original dimensions
    size_t orig_width = MagickGetImageWidth(wand);
    size_t orig_height = MagickGetImageHeight(wand);

    // STEP 2: Calculate scaling to fit in 22x22 (leaving 3px padding on each
    // side) This gives more breathing room around the letter
    size_t target_size = 22;  // Target size with padding margin
    double scale_w = (double)target_size / orig_width;
    double scale_h = (double)target_size / orig_height;
    double scale = (scale_w < scale_h) ? scale_w : scale_h;

    // Don't scale up, only scale down
    if (scale > 1.0) { scale = 1.0; }

    size_t new_width = (size_t)(orig_width * scale);
    size_t new_height = (size_t)(orig_height * scale);

    // Make sure it fits in target_size
    if (new_width > target_size) new_width = target_size;
    if (new_height > target_size) new_height = target_size;

    // STEP 3: Resize if needed
    if (new_width != orig_width || new_height != orig_height)
    {
        status = MagickResizeImage(wand, new_width, new_height, LanczosFilter);
        if (status == MagickFalse)
        {
            fprintf(stderr, "Error resizing image %s\n", filepath);
            DestroyMagickWand(wand);
            return NULL;
        }
    }

    // STEP 4: Create new 28x28 image with white background
    MagickWand *canvas = NewMagickWand();
    PixelWand *white = NewPixelWand();
    PixelSetColor(white, "white");
    MagickNewImage(canvas, 28, 28, white);
    DestroyPixelWand(white);

    // STEP 5: Center the resized image with padding on all sides
    ssize_t x_offset = (28 - new_width) / 2;
    ssize_t y_offset = (28 - new_height) / 2;

    // Composite the resized image onto the white canvas
    MagickCompositeImage(
        canvas, wand, OverCompositeOp, MagickTrue, x_offset, y_offset
    );

    // STEP 6: Export pixels
    uint8_t *pixels = malloc(28 * 28 * sizeof(uint8_t));
    if (!pixels)
    {
        fprintf(stderr, "Error allocating pixel buffer\n");
        DestroyMagickWand(wand);
        DestroyMagickWand(canvas);
        return NULL;
    }

    status =
        MagickExportImagePixels(canvas, 0, 0, 28, 28, "I", CharPixel, pixels);

    if (status == MagickFalse)
    {
        fprintf(stderr, "Error exporting pixels from %s\n", filepath);
        free(pixels);
        DestroyMagickWand(wand);
        DestroyMagickWand(canvas);
        return NULL;
    }

    // NO INVERSION - keep white background, black letters

    DestroyMagickWand(wand);
    DestroyMagickWand(canvas);

    return pixels;
}

// Shuffle dataset (Fisher-Yates algorithm)
static void shuffle_dataset(ImageData *data)
{
    srand(time(NULL));

    for (size_t i = data->num_images - 1; i > 0; i--)
    {
        size_t j = rand() % (i + 1);

        // Swap images
        uint8_t *temp_img = data->images[i];
        data->images[i] = data->images[j];
        data->images[j] = temp_img;

        // Swap labels
        uint8_t temp_label = data->labels[i];
        data->labels[i] = data->labels[j];
        data->labels[j] = temp_label;
    }
}

// Check if file is an image (by extension)
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

// Count images in a directory
static size_t count_images_in_dir(const char *dir_path)
{
    DIR *dir = opendir(dir_path);
    if (!dir) return 0;

    size_t count = 0;
    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG && is_image_file(entry->d_name))
        {
            count++;
        }
    }

    closedir(dir);
    return count;
}

// Load dataset from folder structure (dataset/A/, dataset/B/, ...)
ImageData *load_dataset_from_folders(const char *dataset_path)
{
    MagickWandGenesis();

    printf("Loading dataset from: %s\n", dataset_path);

    // Count total images across all letter folders
    size_t total_images = 0;
    for (char letter = 'A'; letter <= 'Z'; letter++)
    {
        char folder_path[512];
        snprintf(
            folder_path, sizeof(folder_path), "%s/%c", dataset_path, letter
        );

        size_t count = count_images_in_dir(folder_path);
        if (count > 0)
        {
            printf("  %c: %zu images\n", letter, count);
            total_images += count;
        }
    }

    if (total_images == 0)
    {
        fprintf(stderr, "No images found in dataset\n");
        return NULL;
    }

    printf("Total images: %zu\n", total_images);

    // Allocate ImageData structure
    ImageData *data = malloc(sizeof(ImageData));
    if (!data)
    {
        fprintf(stderr, "Error allocating ImageData\n");
        return NULL;
    }
    memset(data, 0, sizeof(ImageData));

    data->num_images = total_images;
    data->width = 28;
    data->height = 28;
    data->images = malloc(total_images * sizeof(uint8_t *));
    data->labels = malloc(total_images * sizeof(uint8_t));

    if (!data->images || !data->labels)
    {
        fprintf(stderr, "Error allocating image/label arrays\n");
        free(data);
        return NULL;
    }

    // Load images from each letter folder
    size_t idx = 0;

    for (char letter = 'A'; letter <= 'Z'; letter++)
    {
        char folder_path[512];
        snprintf(
            folder_path, sizeof(folder_path), "%s/%c", dataset_path, letter
        );

        DIR *dir = opendir(folder_path);
        if (!dir) continue;  // Skip if folder doesn't exist

        uint8_t label = letter - 'A';  // A=0, B=1, ..., Z=25
        struct dirent *entry;
        size_t loaded_in_folder = 0;

        while ((entry = readdir(dir)) != NULL)
        {
            // Skip directories and non-image files
            if (entry->d_type != DT_REG || !is_image_file(entry->d_name))
            {
                continue;
            }

            // Build full file path
            char filepath[768];
            snprintf(
                filepath, sizeof(filepath), "%s/%s", folder_path, entry->d_name
            );

            // Load and resize/pad to 28x28
            uint8_t *pixels = resize_and_pad_to_28x28(filepath);

            if (!pixels)
            {
                fprintf(
                    stderr, "Warning: failed to load %s, skipping\n", filepath
                );
                continue;
            }

            data->images[idx] = pixels;
            data->labels[idx] = label;
            idx++;
            loaded_in_folder++;
        }

        closedir(dir);

        if (loaded_in_folder > 0)
        {
            printf(
                "  ✓ Loaded %zu images for letter %c\n", loaded_in_folder,
                letter
            );
        }
    }

    // Update actual count (in case some images failed to load)
    data->num_images = idx;

    printf("\n✓ Total loaded: %zu images\n", data->num_images);

    // SHUFFLE the dataset
    printf("Shuffling dataset...\n");
    shuffle_dataset(data);
    printf("✓ Dataset shuffled\n");

    return data;
}

// Convert ImageData to Dataset for neural network
void images_to_dataset(
    ImageData *images,
    Dataset **out_dataset,
    size_t num_classes
)
{
    Dataset *dataset = malloc(sizeof(Dataset));
    if (!dataset)
    {
        fprintf(stderr, "Error allocating dataset\n");
        return;
    }

    dataset->num_samples = images->num_images;
    dataset->input_size = images->width * images->height;
    dataset->output_size = num_classes;

    dataset->inputs = malloc(dataset->num_samples * sizeof(double *));
    dataset->targets = malloc(dataset->num_samples * sizeof(double *));

    if (!dataset->inputs || !dataset->targets)
    {
        fprintf(stderr, "Error allocating dataset arrays\n");
        free(dataset);
        return;
    }

    for (int i = 0; i < dataset->num_samples; i++)
    {
        // Normalize pixels from [0, 255] to [0, 1]
        dataset->inputs[i] = malloc(dataset->input_size * sizeof(double));
        if (!dataset->inputs[i])
        {
            fprintf(stderr, "Error allocating input %d\n", i);
            for (int j = 0; j < i; j++)
            {
                free(dataset->inputs[j]);
                free(dataset->targets[j]);
            }
            free(dataset->inputs);
            free(dataset->targets);
            free(dataset);
            return;
        }

        for (size_t j = 0; j < dataset->input_size; j++)
        {
            dataset->inputs[i][j] = images->images[i][j] / 255.0;
        }

        // One-hot encode labels
        dataset->targets[i] = calloc(num_classes, sizeof(double));
        if (!dataset->targets[i])
        {
            fprintf(stderr, "Error allocating target %d\n", i);
            for (int j = 0; j <= i; j++) { free(dataset->inputs[j]); }
            for (int j = 0; j < i; j++) { free(dataset->targets[j]); }
            free(dataset->inputs);
            free(dataset->targets);
            free(dataset);
            return;
        }

        if (images->labels[i] < num_classes)
        {
            dataset->targets[i][images->labels[i]] = 1.0;
        }
        else
        {
            fprintf(
                stderr, "Warning: invalid label %d at sample %d (max: %zu)\n",
                images->labels[i], i, num_classes - 1
            );
        }
    }

    *out_dataset = dataset;
    printf(
        "✓ Converted to dataset: %d samples, input_size=%zu, output_size=%zu\n",
        dataset->num_samples, dataset->input_size, dataset->output_size
    );
}

// Free image data
void free_image_data(ImageData *data)
{
    if (!data) return;

    if (data->images)
    {
        for (size_t i = 0; i < data->num_images; i++)
        {
            if (data->images[i]) { free(data->images[i]); }
        }
        free(data->images);
    }

    if (data->labels) { free(data->labels); }

    free(data);

    MagickWandTerminus();
}
