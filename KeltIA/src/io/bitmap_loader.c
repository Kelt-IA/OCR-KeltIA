#include "../../include/io/bitmap_loader.h"
#include "../../include/nn/include_nn.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    // INVERT pixels: white background -> black, black letters -> white
    // This matches MNIST/EMNIST format (black background, white letters)
    for (size_t i = 0; i < image_size; i++) { pixels[i] = 255 - pixels[i]; }

    DestroyMagickWand(wand);
    return pixels;
}

// Resize image to target dimensions using MagickWand
uint8_t *resize_image_magick(
    const char *filepath,
    size_t target_width,
    size_t target_height
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

    // Resize image (using Lanczos filter for quality)
    status =
        MagickResizeImage(wand, target_width, target_height, LanczosFilter);
    if (status == MagickFalse)
    {
        fprintf(stderr, "Error resizing image %s\n", filepath);
        DestroyMagickWand(wand);
        return NULL;
    }

    // Export pixels
    size_t image_size = target_width * target_height;
    uint8_t *pixels = malloc(image_size * sizeof(uint8_t));

    if (!pixels)
    {
        fprintf(stderr, "Error allocating pixel buffer\n");
        DestroyMagickWand(wand);
        return NULL;
    }

    status = MagickExportImagePixels(
        wand, 0, 0, target_width, target_height, "I", CharPixel, pixels
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

// Load multiple images from directory with labels file
ImageData *
load_images_magick(const char *directory_path, const char *label_file)
{
    // Initialize MagickWand environment
    MagickWandGenesis();

    ImageData *data = malloc(sizeof(ImageData));
    if (!data)
    {
        fprintf(stderr, "Error allocating ImageData\n");
        return NULL;
    }
    memset(data, 0, sizeof(ImageData));

    // Read labels file (format: filename,label per line)
    FILE *label_f = fopen(label_file, "r");
    if (!label_f)
    {
        fprintf(stderr, "Error opening label file: %s\n", label_file);
        free(data);
        return NULL;
    }

    // Count number of images
    char line[512];
    size_t count = 0;
    while (fgets(line, sizeof(line), label_f))
    {
        if (line[0] != '\n' && line[0] != '#')
        {  // Skip empty lines and comments
            count++;
        }
    }
    rewind(label_f);

    if (count == 0)
    {
        fprintf(stderr, "No images found in label file\n");
        fclose(label_f);
        free(data);
        return NULL;
    }

    printf("Loading %zu images from %s\n", count, directory_path);

    // Allocate arrays
    data->num_images = count;
    data->images = malloc(count * sizeof(uint8_t *));
    data->labels = malloc(count * sizeof(uint8_t));

    if (!data->images || !data->labels)
    {
        fprintf(stderr, "Error allocating image/label arrays\n");
        fclose(label_f);
        free(data);
        return NULL;
    }

    // Load each image
    size_t idx = 0;
    size_t first_width = 0, first_height = 0;

    while (fgets(line, sizeof(line), label_f) && idx < count)
    {
        // Skip empty lines and comments
        if (line[0] == '\n' || line[0] == '#') continue;

        // Parse filename and label
        char filename[256];
        int label;

        if (sscanf(line, "%255[^,],%d", filename, &label) != 2)
        {
            fprintf(
                stderr, "Warning: invalid line format at line %zu\n", idx + 1
            );
            continue;
        }

        // Build full path
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", directory_path, filename);

        // Load image
        size_t width, height;
        uint8_t *image = load_single_image_magick(filepath, &width, &height);

        if (!image)
        {
            fprintf(stderr, "Warning: failed to load %s, skipping\n", filepath);
            continue;
        }

        // Store dimensions from first image
        if (idx == 0)
        {
            first_width = width;
            first_height = height;
            data->width = width;
            data->height = height;
            printf("Image dimensions: %zux%zu pixels\n", width, height);
        }
        else
        {
            // Verify all images have same dimensions
            if (width != first_width || height != first_height)
            {
                fprintf(
                    stderr,
                    "Warning: image %s has different dimensions (%zux%zu), "
                    "expected %zux%zu. Resizing...\n",
                    filename, width, height, first_width, first_height
                );
                free(image);

                // Resize to match first image dimensions
                image =
                    resize_image_magick(filepath, first_width, first_height);
                if (!image)
                {
                    fprintf(
                        stderr, "Error resizing image %s, skipping\n", filename
                    );
                    continue;
                }
            }
        }

        data->images[idx] = image;
        data->labels[idx] = (uint8_t)label;
        idx++;

        if ((idx % 100) == 0)
        {
            printf("Loaded %zu/%zu images...\n", idx, count);
        }
    }

    fclose(label_f);

    data->num_images = idx;  // Update to actual loaded count

    printf(
        "✓ Successfully loaded %zu images (%zux%zu)\n", data->num_images,
        data->width, data->height
    );

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
            // Cleanup
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
            // Cleanup
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

    // Cleanup MagickWand environment
    MagickWandTerminus();
}
