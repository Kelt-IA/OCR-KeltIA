#include "../../include/detect_zones/detect_char.h"
#include "../../include/detect_zones/detect_zones.h"
#include <stdlib.h>
#include <string.h>

// Helper function to calculate median width
static int calculate_median_width(CharBBox *characters, int count)
{
    if (count == 0) return 0;

    // Copy widths to array for sorting
    int *widths = malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) { widths[i] = characters[i].w; }

    // Simple bubble sort (good enough for small arrays)
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (widths[j] > widths[j + 1])
            {
                int temp = widths[j];
                widths[j] = widths[j + 1];
                widths[j + 1] = temp;
            }
        }
    }

    // Get median
    int median = widths[count / 2];
    free(widths);

    return median;
}

// Helper function to calculate median height
static int calculate_median_height(CharBBox *characters, int count)
{
    if (count == 0) return 0;

    // Copy heights to array for sorting
    int *heights = malloc(count * sizeof(int));
    for (int i = 0; i < count; i++) { heights[i] = characters[i].h; }

    // Simple bubble sort
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (heights[j] > heights[j + 1])
            {
                int temp = heights[j];
                heights[j] = heights[j + 1];
                heights[j + 1] = temp;
            }
        }
    }

    // Get median
    int median = heights[count / 2];
    free(heights);

    return median;
}

// Remove tiny noise characters that are way smaller than median
static CharBBox *remove_tiny_characters(
    CharBBox *characters,
    int *char_count,
    float min_size_ratio
)
{
    if (*char_count == 0) return characters;

    // Calculate median width and height
    int median_width = calculate_median_width(characters, *char_count);
    int median_height = calculate_median_height(characters, *char_count);

    if (median_width == 0 || median_height == 0) return characters;

    int min_width = (int)(median_width * min_size_ratio);
    int min_height = (int)(median_height * min_size_ratio);

    printf("  Median size: %dx%d pixels\n", median_width, median_height);
    printf(
        "  Minimum size threshold: %dx%d pixels (%.0f%% of median)\n",
        min_width, min_height, min_size_ratio * 100
    );

    // Count how many valid characters we have
    int valid_count = 0;
    for (int i = 0; i < *char_count; i++)
    {
        if (characters[i].w >= min_width && characters[i].h >= min_height)
        {
            valid_count++;
        }
        else
        {
            printf(
                "  Removing tiny char #%d: size %dx%d (too small)\n", i,
                characters[i].w, characters[i].h
            );
        }
    }

    if (valid_count == *char_count)
    {
        // No characters to remove
        return characters;
    }

    // Create new array with only valid characters
    CharBBox *filtered = malloc(valid_count * sizeof(CharBBox));
    int write_index = 0;

    for (int i = 0; i < *char_count; i++)
    {
        if (characters[i].w >= min_width && characters[i].h >= min_height)
        {
            filtered[write_index] = characters[i];
            write_index++;
        }
    }

    free(characters);
    *char_count = valid_count;

    return filtered;
}

// Split wide characters that are likely grouped letters
static CharBBox *split_grouped_letters(
    CharBBox *characters,
    int *char_count,
    float threshold_multiplier
)
{
    if (*char_count == 0) return characters;

    // Calculate median width
    int median_width = calculate_median_width(characters, *char_count);

    if (median_width == 0) return characters;

    printf("  Median width: %d pixels\n", median_width);
    printf(
        "  Threshold for splitting: %.1f x median = %.1f pixels\n",
        threshold_multiplier, threshold_multiplier * median_width
    );

    // Count how many splits we need
    int new_count = *char_count;
    for (int i = 0; i < *char_count; i++)
    {
        float width_ratio = (float)characters[i].w / (float)median_width;
        if (width_ratio > threshold_multiplier)
        {
            // Estimate number of letters in this group
            int estimated_letters = (int)(width_ratio + 0.5);  // Round
            if (estimated_letters < 2) estimated_letters = 2;

            // Add extra slots for split letters (minus 1 because original
            // exists)
            new_count += (estimated_letters - 1);
        }
    }

    // Allocate new array if we need splits
    if (new_count > *char_count)
    {
        CharBBox *new_characters = malloc(new_count * sizeof(CharBBox));
        int write_index = 0;

        for (int i = 0; i < *char_count; i++)
        {
            float width_ratio = (float)characters[i].w / (float)median_width;

            if (width_ratio > threshold_multiplier)
            {
                // Split this character
                int estimated_letters = (int)(width_ratio + 0.5);
                if (estimated_letters < 2) estimated_letters = 2;

                int sub_width = characters[i].w / estimated_letters;

                printf(
                    "  Splitting char #%d: width=%d (%.1fx median) into %d "
                    "letters of %d pixels each\n",
                    i, characters[i].w, width_ratio, estimated_letters,
                    sub_width
                );

                // Create sub-bboxes
                for (int j = 0; j < estimated_letters; j++)
                {
                    new_characters[write_index].x =
                        characters[i].x + (j * sub_width);
                    new_characters[write_index].y = characters[i].y;
                    new_characters[write_index].w = sub_width;
                    new_characters[write_index].h = characters[i].h;
                    write_index++;
                }
            }
            else
            {
                // Keep original
                new_characters[write_index] = characters[i];
                write_index++;
            }
        }

        free(characters);
        *char_count = write_index;
        return new_characters;
    }

    return characters;
}

void flood_fill(
    MagickWand *wand,
    int start_x,
    int start_y,
    char *visited,
    int width,
    int height,
    int x_offset,
    int y_offset,
    int *x_min,
    int *x_max,
    int *y_min,
    int *y_max
)
{
    Point *stack = malloc(width * height * sizeof(Point));
    size_t stack_top = 0;

    PixelWand *pixel_wand = NewPixelWand();

    // Initialize stack with starting point
    stack[stack_top].x = start_x;
    stack[stack_top].y = start_y;
    stack_top++;

    int local_x = start_x - x_offset;
    int local_y = start_y - y_offset;
    visited[local_y * width + local_x] = 1;

    // Process all points in stack
    while (stack_top > 0)
    {
        stack_top--;
        int abs_x = stack[stack_top].x;
        int abs_y = stack[stack_top].y;

        int local_x = abs_x - x_offset;
        int local_y = abs_y - y_offset;

        // Update bounding box size
        if (local_x < *x_min) *x_min = local_x;
        if (local_x > *x_max) *x_max = local_x;
        if (local_y < *y_min) *y_min = local_y;
        if (local_y > *y_max) *y_max = local_y;

        // Check connected pixels
        for (int dy = -1; dy <= 1; dy++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                if (dx == 0 && dy == 0) continue;

                int new_abs_x = abs_x + dx;
                int new_abs_y = abs_y + dy;
                int new_local_x = new_abs_x - x_offset;
                int new_local_y = new_abs_y - y_offset;

                // Check sides (local)
                if (new_local_x < 0 || new_local_x >= width ||
                    new_local_y < 0 || new_local_y >= height)
                    continue;
                if (visited[new_local_y * width + new_local_x]) continue;

                // Add to stack if black pixel found (absolutes)
                if (is_pixel_black(wand, new_abs_x, new_abs_y, pixel_wand))
                {
                    visited[new_local_y * width + new_local_x] = 1;
                    stack[stack_top].x = new_abs_x;
                    stack[stack_top].y = new_abs_y;
                    stack_top++;
                }
            }
        }
    }

    free(stack);
    DestroyPixelWand(pixel_wand);
}

// Detect all characters in a zone
CharBBox *detect_characters(
    MagickWand *wand,
    int x_offset,
    int y_offset,
    int width,
    int height,
    int *char_count
)
{
    char *visited = calloc(width * height, sizeof(char));
    CharBBox *characters = malloc(1024 * sizeof(CharBBox));
    *char_count = 0;

    PixelWand *pixel_wand = NewPixelWand();

    // Scan all pixels in zone
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int abs_x = x_offset + x;
            int abs_y = y_offset + y;

            if (visited[y * width + x]) continue;

            if (is_pixel_black(wand, abs_x, abs_y, pixel_wand))
            {
                int x_min = x, x_max = x, y_min = y, y_max = y;

                // Call flood fill to find connected black pixels
                visited[y * width + x] = 1;
                flood_fill(
                    wand, abs_x, abs_y, visited, width, height, x_offset,
                    y_offset, &x_min, &x_max, &y_min, &y_max
                );

                int char_width = x_max - x_min + 1;
                int char_height = y_max - y_min + 1;

                // Just in case there is still noise
                if (char_width > 2 && char_height > 2 && *char_count < 1024)
                {
                    characters[*char_count].x = x_min + x_offset;
                    characters[*char_count].y = y_min + y_offset;
                    characters[*char_count].w = char_width;
                    characters[*char_count].h = char_height;

                    (*char_count)++;
                }
            }
        }
    }

    // Check if first item is grid
    if (*char_count > 1 && characters[0].h > characters[1].h * 3)
    {
        memmove(
            &characters[0], &characters[1],
            ((--(*char_count)) * sizeof(CharBBox))
        );
    }

    free(visited);
    DestroyPixelWand(pixel_wand);

    // POST-PROCESSING 1: Remove tiny noise characters
    printf("\n  === Post-processing: Remove Tiny Characters ===\n");
    printf("  Before filtering: %d characters\n", *char_count);
    characters =
        remove_tiny_characters(characters, char_count, 0.20);  // 20% threshold
    printf("  After filtering: %d characters\n", *char_count);

    // POST-PROCESSING 2: Split grouped letters
    printf("\n  === Post-processing: Split Grouped Letters ===\n");
    printf("  Before splitting: %d characters\n", *char_count);
    characters = split_grouped_letters(characters, char_count, 1.8);
    printf("  After splitting: %d characters\n\n", *char_count);

    return characters;
}

// Export a zone containing a character
MagickWand *extract_zone(MagickWand *wand, int x, int y, int width, int height)
{
    MagickWand *region = CloneMagickWand(wand);
    MagickCropImage(region, width, height, x, y);
    return region;
}

void DrawLetterBoundries(
    MagickWand *wand,
    DrawingWand *draw,
    CharBBox *grid_characters,
    int grid_chars,
    char *color
)
{
    PixelWand *stroke_color = NewPixelWand();
    PixelWand *fill_color = NewPixelWand();

    PixelSetColor(fill_color, "none");

    PixelSetColor(stroke_color, color);
    DrawSetStrokeColor(draw, stroke_color);
    DrawSetStrokeWidth(draw, 2);

    PixelSetColor(fill_color, "none");
    DrawSetFillColor(draw, fill_color);

    for (int i = 0; i < grid_chars; i++)
    {
        DrawRectangle(
            draw, grid_characters[i].x, grid_characters[i].y,
            grid_characters[i].x + grid_characters[i].w,
            grid_characters[i].y + grid_characters[i].h
        );
    }

    DestroyPixelWand(stroke_color);
    DestroyPixelWand(fill_color);
    MagickDrawImage(wand, draw);
}

int save_charbbox_as_bitmap(MagickWand *wand, CharBBox bbox, const char *path)
{
    const int TARGET_SIZE = 28;

    // extract zone of char
    MagickWand *letter = extract_zone(wand, bbox.x, bbox.y, bbox.w, bbox.h);
    if (letter == NULL) return -1;

    // get dimensions
    size_t orig_width = MagickGetImageWidth(letter);
    size_t orig_height = MagickGetImageHeight(letter);

    // calculate scaling
    double scale_w = (double)TARGET_SIZE / orig_width;
    double scale_h = (double)TARGET_SIZE / orig_height;

    double scale = fmin(scale_w, scale_h);

    size_t new_width = (size_t)(orig_width * scale);
    size_t new_height = (size_t)(orig_height * scale);

    // Resize only if necessary
    if (new_width != orig_width || new_height != orig_height)
    {
        MagickResizeImage(letter, new_width, new_height, LanczosFilter);
    }

    // background white
    PixelWand *bg_color = NewPixelWand();
    PixelSetColor(bg_color, "white");
    MagickSetImageBackgroundColor(letter, bg_color);
    DestroyPixelWand(bg_color);

    MagickSetGravity(letter, CenterGravity);

    ssize_t offset_x = (TARGET_SIZE - new_width) / 2;
    ssize_t offset_y = (TARGET_SIZE - new_height) / 2;

    MagickExtentImage(letter, TARGET_SIZE, TARGET_SIZE, -offset_x, -offset_y);

    MagickBooleanType status = MagickWriteImage(letter, path);

    DestroyMagickWand(letter);

    return (status == MagickTrue) ? 0 : -1;
}

double *charbbox_to_cnn_input(MagickWand *wand, CharBBox bbox)
{
    const int TARGET_SIZE = 28;

    // Extract zone of char
    MagickWand *letter = extract_zone(wand, bbox.x, bbox.y, bbox.w, bbox.h);
    if (letter == NULL) return NULL;

    // Get dimensions
    size_t orig_width = MagickGetImageWidth(letter);
    size_t orig_height = MagickGetImageHeight(letter);

    // Calculate scaling
    double scale_w = (double)TARGET_SIZE / orig_width;
    double scale_h = (double)TARGET_SIZE / orig_height;
    double scale = fmin(scale_w, scale_h);

    size_t new_width = (size_t)(orig_width * scale);
    size_t new_height = (size_t)(orig_height * scale);

    // Resize only if necessary
    if (new_width != orig_width || new_height != orig_height)
    {
        MagickResizeImage(letter, new_width, new_height, LanczosFilter);
    }

    // White background
    PixelWand *bg_color = NewPixelWand();
    PixelSetColor(bg_color, "white");
    MagickSetImageBackgroundColor(letter, bg_color);
    DestroyPixelWand(bg_color);

    MagickSetGravity(letter, CenterGravity);

    ssize_t offset_x = (TARGET_SIZE - new_width) / 2;
    ssize_t offset_y = (TARGET_SIZE - new_height) / 2;

    MagickExtentImage(letter, TARGET_SIZE, TARGET_SIZE, -offset_x, -offset_y);

    // Export pixels to array
    uint8_t *pixels = malloc(TARGET_SIZE * TARGET_SIZE * sizeof(uint8_t));
    if (!pixels)
    {
        DestroyMagickWand(letter);
        return NULL;
    }

    MagickBooleanType status = MagickExportImagePixels(
        letter, 0, 0, TARGET_SIZE, TARGET_SIZE, "I", CharPixel, pixels
    );

    DestroyMagickWand(letter);

    if (status == MagickFalse)
    {
        free(pixels);
        return NULL;
    }

    // Normalize to [0, 1]
    double *normalized = malloc(TARGET_SIZE * TARGET_SIZE * sizeof(double));
    if (!normalized)
    {
        free(pixels);
        return NULL;
    }

    for (int i = 0; i < TARGET_SIZE * TARGET_SIZE; i++)
    {
        normalized[i] = pixels[i] / 255.0;
    }

    free(pixels);

    return normalized;
}
