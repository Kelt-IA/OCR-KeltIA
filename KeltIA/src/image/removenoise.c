#include "../../include/image/removenoise.h"

// Estimation of the variance of grey levels
double compute_gray_stddev(MagickWand *wand)
{
    PixelIterator *iterator;
    PixelWand **pixels;
    size_t width, height;
    height = MagickGetImageHeight(wand);
    width = MagickGetImageWidth(wand);

    iterator = NewPixelIterator(wand);
    if (iterator == NULL) return 0.0;

    double sum = 0.0;
    double sumsq = 0.0;
    size_t count = 0;

    for (size_t y = 0; y < height; y++)
    {
        pixels = PixelGetNextIteratorRow(iterator, &width);
        if (pixels == NULL) break;

        for (size_t x = 0; x < width; x++)
        {
            double g = PixelGetRed(pixels[x]);  // image in grayscale
            double val = g * 255.0;
            sum += val;
            sumsq += val * val;
            count++;
        }
    }

    iterator = DestroyPixelIterator(iterator);

    if (count == 0) return 0.0;

    double mean = sum / count;
    double variance = (sumsq / count) - (mean * mean);
    double stddev = sqrt(fabs(variance));

    return stddev;
}

// Automatic radius selection based on noise level
double auto_radius(double stddev)
{
    if (stddev < 10) return 0.5;  // very clean image
    if (stddev < 25) return 1.0;  // slight noise
    if (stddev < 50) return 1.5;  // medium noise
    if (stddev < 80) return 2.0;  // strong noise
    return 2.5;                   // very strong noise
}

// Detect fine lines/edges using Laplacian operator (low-pass filter)
// Returns a score [0,1]: higher = more fine details detected
double detect_fine_lines(MagickWand *wand)
{
    if (!wand) return 0.5;

    MagickWand *edges = CloneMagickWand(wand);
    if (!edges) return 0.5;

    // Apply edge detection (Laplacian approximation)
    if (MagickEdgeImage(edges, 2.0) == MagickFalse)
    {
        DestroyMagickWand(edges);
        return 0.5;
    }

    // Count bright pixels (edges) in the result
    PixelIterator *iterator = NewPixelIterator(edges);
    if (!iterator)
    {
        DestroyMagickWand(edges);
        return 0.5;
    }

    PixelWand **pixels;
    size_t width = MagickGetImageWidth(edges);
    size_t height = MagickGetImageHeight(edges);
    size_t edge_count = 0;
    size_t total = width * height;

    for (size_t y = 0; y < height; y++)
    {
        pixels = PixelGetNextIteratorRow(iterator, &width);
        if (pixels == NULL) break;

        for (size_t x = 0; x < width; x++)
        {
            double intensity = PixelGetRed(pixels[x]) * 255.0;
            if (intensity > 50.0)  // threshold for edge detection
                edge_count++;
        }
    }

    iterator = DestroyPixelIterator(iterator);
    DestroyMagickWand(edges);

    // Return proportion of edge pixels (0-1)
    double edge_ratio = (double)edge_count / (double)total;
    return edge_ratio;
}

// Auto-compute optimal denoising strength based on noise level and fine lines
double auto_strength(MagickWand *wand)
{
    if (!wand) return 0.5;

    double stddev = compute_gray_stddev(wand);
    double edge_score = detect_fine_lines(wand);

    // Base strength on noise level
    double base_strength =
        auto_radius(stddev) / 2.0;  // normalize to [0.25-1.25]

    // Reduce strength if fine lines detected (preserve grid)
    double adjusted = base_strength * (1.0 - edge_score * 0.7);

    // Clamp to safe range
    if (adjusted < 0.1) adjusted = 0.1;
    if (adjusted > 2.0) adjusted = 2.0;

    printf(
        "→ Edge density: %.2f%% → adjusted strength: %.2f (base: %.2f)\n",
        edge_score * 100.0, adjusted, base_strength
    );

    return adjusted;
}

// Adaptive denoising with optional stronger pass and despeckle
MagickWand *remove_noise_adaptive(MagickWand *wand, double strength)
{
    if (!wand) return NULL;

    // Clamp strength to avoid destroying details
    if (strength < 0.5) strength = 0.5;
    if (strength > 2.0) strength = 2.0;

    MagickSetImageType(wand, GrayscaleType);

    double stddev_before = compute_gray_stddev(wand);
    double radius = auto_radius(stddev_before) * strength;

    printf(
        "→ Estimated noise = %.2f → radius=%.2f (strength=%.2f)\n",
        stddev_before, radius, strength
    );

    MagickWand *clean = CloneMagickWand(wand);
    if (MagickStatisticImage(clean, MedianStatistic, radius, radius) ==
        MagickFalse)
    {
        fprintf(stderr, "Error: failed to apply median filter.\n");
        DestroyMagickWand(clean);
        return NULL;
    }

    // Re-estimate after first pass; if still noisy and strength high, add a
    // light second pass
    double stddev_after = compute_gray_stddev(clean);
    if (stddev_after > 35.0 && strength > 1.0)
    {
        double radius2 = radius + 0.5;
        if (radius2 > 3.5) radius2 = 3.5;

        if (MagickStatisticImage(clean, MedianStatistic, radius2, radius2) ==
            MagickFalse)
        {
            fprintf(stderr, "Error: failed to apply second median pass.\n");
            DestroyMagickWand(clean);
            return NULL;
        }
    }

    // Optional despeckle for isolated pixels when noise remains
    if (stddev_after > 20.0 && strength >= 1.2)
    {
        if (MagickDespeckleImage(clean) == MagickFalse)
        {
            fprintf(stderr, "Warning: despeckle failed.\n");
        }
    }

    return clean;
}

// Reading and writing
MagickWand *read_image(const char *path)
{
    MagickWand *wand = NewMagickWand();
    if (MagickReadImage(wand, path) == MagickFalse)
    {
        fprintf(stderr, "Error: cannot read '%s'\n", path);
        DestroyMagickWand(wand);
        return NULL;
    }
    return wand;
}

int write_image(MagickWand *wand, const char *path)
{
    if (!wand) return 0;
    if (MagickWriteImage(wand, path) == MagickFalse) return 0;
    return 1;
}

// Main function of automatic noise reduction
MagickWand *remove_noise(MagickWand *wand)
{
    // Default strength = 1.0 keeps legacy behavior while allowing adaptive
    // tuning
    return remove_noise_adaptive(wand, 1.0);
}

// Clean binary image by removing isolated pixels
// Remove a black pixel if it has fewer than N black neighbors
// Apply multiple passes for progressive noise removal
MagickWand *clean_binary_image(MagickWand *wand)
{
    if (!wand) return NULL;

    size_t width = MagickGetImageWidth(wand);
    size_t height = MagickGetImageHeight(wand);

    MagickWand *clean = CloneMagickWand(wand);
    if (!clean) return NULL;

    size_t total_removed = 0;

    // Use threshold of 3 for better noise removal when no fine grid lines
    // present
    int threshold = 3;

    // Apply 2 passes for progressive cleaning
    for (int pass = 0; pass < 2; pass++)
    {
        // Allocate array to store pixel values (1 = black, 0 = white)
        unsigned char *pixels = calloc(width * height, sizeof(unsigned char));
        if (!pixels)
        {
            fprintf(stderr, "Error: failed to allocate pixel buffer.\n");
            return clean;
        }

        // Read all pixels into buffer
        PixelIterator *iter = NewPixelIterator(clean);
        if (!iter)
        {
            free(pixels);
            return clean;
        }

        PixelWand **row;
        size_t row_width;
        for (size_t y = 0; y < height; y++)
        {
            row = PixelGetNextIteratorRow(iter, &row_width);
            if (!row) break;

            for (size_t x = 0; x < row_width; x++)
            {
                double intensity = PixelGetRed(row[x]);
                pixels[y * width + x] = (intensity < 0.5) ? 1 : 0;
            }
        }
        DestroyPixelIterator(iter);

        // Process pixels: remove isolated ones
        size_t removed = 0;
        iter = NewPixelIterator(clean);
        if (!iter)
        {
            free(pixels);
            return clean;
        }

        for (size_t y = 0; y < height; y++)
        {
            row = PixelGetNextIteratorRow(iter, &row_width);
            if (!row) break;

            for (size_t x = 0; x < row_width; x++)
            {
                size_t idx = y * width + x;
                if (pixels[idx] == 0) continue;

                int neighbors = 0;
                int left = 0, right = 0, top = 0, bottom = 0;

                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dx = -1; dx <= 1; dx++)
                    {
                        if (dy == 0 && dx == 0) continue;

                        int ny = (int)y + dy;
                        int nx = (int)x + dx;

                        if (ny >= 0 && ny < (int)height && nx >= 0 &&
                            nx < (int)width)
                        {
                            if (pixels[ny * width + nx] == 1)
                            {
                                neighbors++;
                                if (dy == 0 && dx == -1) left = 1;
                                if (dy == 0 && dx == 1) right = 1;
                                if (dy == -1 && dx == 0) top = 1;
                                if (dy == 1 && dx == 0) bottom = 1;
                            }
                        }
                    }
                }

                // Check if pixel is part of a line (for grid preservation)
                int is_horizontal_line = (left && right);
                int is_vertical_line = (top && bottom);
                int is_grid_line = is_horizontal_line || is_vertical_line;

                // Remove pixel if below threshold and not part of grid line
                if (neighbors < threshold && !is_grid_line)
                {
                    PixelSetColor(row[x], "white");
                    removed++;
                }
            }

            PixelSyncIterator(iter);
        }

        DestroyPixelIterator(iter);
        free(pixels);

        total_removed += removed;
        if (removed == 0) break;  // Stop early if no changes
    }

    printf(
        "→ Removed %zu isolated pixels in 2 passes (< %d black neighbors).\n",
        total_removed, threshold
    );

    // Post-processing: detect thin vertical segments (16x2 pixels ~ letters
    // like 'i') and add a column of black pixels next to them
    size_t thickened = 0;
    unsigned char *pixels = calloc(width * height, sizeof(unsigned char));
    if (!pixels) return clean;

    // Read current state
    PixelIterator *iter = NewPixelIterator(clean);
    if (iter)
    {
        PixelWand **row;
        size_t row_width;
        for (size_t y = 0; y < height; y++)
        {
            row = PixelGetNextIteratorRow(iter, &row_width);
            if (!row) break;
            for (size_t x = 0; x < row_width; x++)
            {
                double intensity = PixelGetRed(row[x]);
                pixels[y * width + x] = (intensity < 0.5) ? 1 : 0;
            }
        }
        DestroyPixelIterator(iter);
    }

    // Detect vertical segments of height ~16 pixels and width 1-2 pixels
    unsigned char *to_thicken = calloc(width * height, sizeof(unsigned char));
    if (to_thicken)
    {
        for (size_t x = 0; x < width - 3; x++)
        {
            for (size_t y = 0; y < height - 20; y++)
            {
                // Check if we have a vertical black segment starting here
                int col1_height = 0;
                int col2_height = 0;

                // Count consecutive black pixels in column x and x+1
                for (size_t dy = 0; dy < 20 && y + dy < height; dy++)
                {
                    if (pixels[(y + dy) * width + x] == 1)
                        col1_height++;
                    else
                        break;
                }

                for (size_t dy = 0; dy < 20 && y + dy < height; dy++)
                {
                    if (pixels[(y + dy) * width + x + 1] == 1)
                        col2_height++;
                    else
                        break;
                }

                // Case 1: Two-pixel-wide stroke (both columns black)
                if (col1_height >= 12 && col2_height >= 12)
                {
                    int col3_black = 0;
                    for (size_t dy = 0;
                         dy < (size_t)col1_height && y + dy < height; dy++)
                    {
                        if (pixels[(y + dy) * width + x + 2] == 1) col3_black++;
                    }

                    // If third column is mostly white (thin segment detected)
                    if (col3_black < col1_height / 3)
                    {
                        // Mark column x+2 to be thickened
                        for (size_t dy = 0;
                             dy < (size_t)col1_height && y + dy < height; dy++)
                        {
                            to_thicken[(y + dy) * width + x + 2] = 1;
                        }
                    }
                }
                // Case 2: One-pixel-wide stroke (only first column black)
                else if (col1_height >= 12 && col2_height < col1_height / 2)
                {
                    // Verify it's really a thin vertical stroke (check x+2 is
                    // also mostly white)
                    int col3_black = 0;
                    for (size_t dy = 0;
                         dy < (size_t)col1_height && y + dy < height; dy++)
                    {
                        if (x + 2 < width &&
                            pixels[(y + dy) * width + x + 2] == 1)
                            col3_black++;
                    }

                    if (col3_black < col1_height / 3)
                    {
                        // Thicken by adding columns x+1 AND x+2
                        for (size_t dy = 0;
                             dy < (size_t)col1_height && y + dy < height; dy++)
                        {
                            if (x + 1 < width)
                                to_thicken[(y + dy) * width + x + 1] = 1;
                            if (x + 2 < width)
                                to_thicken[(y + dy) * width + x + 2] = 1;
                        }
                    }
                }
            }
        }

        // Apply thickening
        iter = NewPixelIterator(clean);
        if (iter)
        {
            PixelWand **row;
            size_t row_width;
            for (size_t y = 0; y < height; y++)
            {
                row = PixelGetNextIteratorRow(iter, &row_width);
                if (!row) break;

                for (size_t x = 0; x < row_width; x++)
                {
                    if (to_thicken[y * width + x] == 1)
                    {
                        PixelSetColor(row[x], "black");
                        thickened++;
                    }
                }
                PixelSyncIterator(iter);
            }
            DestroyPixelIterator(iter);
        }
        free(to_thicken);
    }
    free(pixels);

    if (thickened > 0)
        printf(
            "→ Thickened %zu pixels in thin vertical segments (~16x2, for "
            "OCR).\n",
            thickened
        );

    return clean;
}
