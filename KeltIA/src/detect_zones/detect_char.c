#include "../../include/detect_zones/detect_char.h"
#include "../../include/detect_zones/detect_zones.h"

void flood_fill(
    MagickWand *wand,
    int start_x,
    int start_y,
    char *visited,
    int width,
    int height,
    int *x_min,
    int *x_max,
    int *y_min,
    int *y_max
)
{
    // This is a simple implementation of a stack
    // Not memory efficient, could be improved later
    Point *stack = malloc(width * height * sizeof(Point));
    size_t stack_top = 0;

    PixelWand *pixel_wand = NewPixelWand();

    // Initialize stack with starting point
    stack[stack_top].x = start_x;
    stack[stack_top].y = start_y;
    stack_top++;

    visited[start_y * width + start_x] = 1;

    // Process all points in stack
    while (stack_top > 0)
    {
        stack_top--;
        int x = stack[stack_top].x;
        int y = stack[stack_top].y;

        // Update bounding box size
        if (x < *x_min) *x_min = x;
        if (x > *x_max) *x_max = x;
        if (y < *y_min) *y_min = y;
        if (y > *y_max) *y_max = y;

        // Check connected pixels
        for (int dy = -1; dy <= 1; dy++)
        {
            for (int dx = -1; dx <= 1; dx++)
            {
                if (dx == 0 && dy == 0) continue;

                int nx = x + dx;
                int ny = y + dy;

                // Check sides
                if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;
                if (visited[ny * width + nx]) continue;

                // Add to stack if black pixel found
                if (is_pixel_black(wand, nx, ny, pixel_wand))
                {
                    visited[ny * width + nx] = 1;
                    stack[stack_top].x = nx;
                    stack[stack_top].y = ny;
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

    // CharBBox buffer, not memory efficient, can be improved
    CharBBox *characters = malloc(1024 * sizeof(CharBBox));
    *char_count = 0;

    PixelWand *pixel_wand = NewPixelWand();

    // Scan all pixels in zone
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            if (visited[y * width + x]) continue;

            if (is_pixel_black(wand, x_offset + x, y_offset + y, pixel_wand))
            {
                int x_min = x, x_max = x, y_min = y, y_max = y;

                // Call flood fill to find connected black pixels
                visited[y * width + x] = 1;
                flood_fill(
                    wand, x, y, visited, width, height, &x_min, &x_max, &y_min,
                    &y_max
                );

                int char_width = x_max - x_min + 1;
                int char_height = y_max - y_min + 1;

                // Just in case there is still noise
                // This allow only char > 4px each side
                if (char_width > 4 && char_height > 4 && *char_count < 1024)
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

    free(visited);
    DestroyPixelWand(pixel_wand);

    return characters;
}

// Export a zone containing a character
MagickWand *extract_zone(MagickWand *wand, int x, int y, int width, int height)
{
    MagickWand *region = CloneMagickWand(wand);
    MagickCropImage(region, width, height, x, y);
    return region;
}
