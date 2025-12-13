#include "../../include/detect_zones/detect_zones.h"
#include <stdio.h>

// Check if a pixel is black (< 128 brightness)
char is_pixel_black(MagickWand *wand, size_t x, size_t y, PixelWand *pixel_wand)
{
    MagickGetImagePixelColor(wand, x, y, pixel_wand);
    return PixelGetRed(pixel_wand) * 255 < 128;
}

// Create horizontal and vertical 1D projections
Projections *projection(MagickWand *wand)
{
    size_t height = MagickGetImageHeight(wand);
    size_t width = MagickGetImageWidth(wand);
    Projections *projs = malloc(sizeof(Projections));
    projs->horizontal = calloc(height, sizeof(int));
    projs->vertical = calloc(width, sizeof(int));
    PixelWand *pixel_wand = NewPixelWand();

    for (size_t y = 0; y < height; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            if (is_pixel_black(wand, x, y, pixel_wand))
            {
                projs->vertical[x]++;
                projs->horizontal[y]++;
            }
        }
    }

    DestroyPixelWand(pixel_wand);
    return projs;
}

// Find two main zones from a projection with adaptive filtering
Zone *find_two_main_zones(int *proj, int size, int *count)
{
    Zone *zones = malloc(sizeof(Zone) * 20);
    *count = 0;

    // Dynamic threshold based on max projection value
    int max_proj = 0;
    long long total_proj = 0;
    for (int i = 0; i < size; i++)
    {
        if (proj[i] > max_proj) max_proj = proj[i];
        total_proj += proj[i];
    }

    // Adaptive threshold: use average as baseline
    int avg_proj = total_proj / size;
    int threshold = avg_proj / 4;  // 25% of average
    if (threshold < 3) threshold = 3;
    if (threshold > max_proj / 10) threshold = max_proj / 10;

    int in_zone = 0;
    int zone_start = 0;
    int zone_density = 0;
    int zone_width = 0;
    int gap = 0;

    // FIX: Adaptive max_gap based on image dimension
    int max_gap = size / 50;  // 2% of dimension instead of fixed 10 pixels
    if (max_gap < 10) max_gap = 10;
    if (max_gap > 50) max_gap = 50;

    for (int i = 0; i < size; i++)
    {
        if (proj[i] > threshold)
        {
            if (!in_zone)
            {
                zone_start = i;
                zone_density = 0;
                zone_width = 0;
                in_zone = 1;
            }
            zone_density += proj[i];
            zone_width++;
            gap = 0;
        }
        else
        {
            if (in_zone)
            {
                gap++;
                zone_width++;
                if (gap > max_gap)
                {
                    // End of zone
                    if (*count < 20)
                    {
                        zones[*count].start = zone_start;
                        zones[*count].end = i - gap;
                        zones[*count].density = zone_density;
                        (*count)++;
                    }
                    in_zone = 0;
                }
            }
        }
    }

    // Handle last zone if exists
    if (in_zone && *count < 20)
    {
        zones[*count].start = zone_start;
        zones[*count].end = size - 1;
        zones[*count].density = zone_density;
        (*count)++;
    }

    // Filter very small zones (< 3% of dimension) only if we have more than 2
    // zones
    if (*count > 2)
    {
        int min_size = size / 33;  // 3% minimum
        int filtered_count = 0;
        for (int i = 0; i < *count; i++)
        {
            int zone_size = zones[i].end - zones[i].start;
            if (zone_size >= min_size)
            {
                zones[filtered_count] = zones[i];
                filtered_count++;
            }
        }
        *count = filtered_count;
    }

    // Keep only the two largest zones by density
    if (*count > 2)
    {
        // Find the two zones with highest density
        for (int i = 0; i < *count - 1; i++)
        {
            for (int j = i + 1; j < *count; j++)
            {
                if (zones[j].density > zones[i].density)
                {
                    Zone temp = zones[i];
                    zones[i] = zones[j];
                    zones[j] = temp;
                }
            }
        }

        if (*count >= 3)
        {
            int second_density = zones[1].density;
            int third_density = zones[2].density;
            if (third_density < second_density * 0.15) { *count = 2; }
        }
        else
        {
            *count = 2;
        }
    }

    // Reorder by position
    if (*count >= 2 && zones[0].start > zones[1].start)
    {
        Zone temp = zones[0];
        zones[0] = zones[1];
        zones[1] = temp;
    }

    Zone *result = malloc(sizeof(Zone) * 2);
    for (int i = 0; i < *count && i < 2; i++) { result[i] = zones[i]; }

    free(zones);
    return result;
}

// Extract horizontal bounding box
BoundingBox extract_bbox_horizontal(MagickWand *wand, int y_min, int y_max)
{
    size_t width = MagickGetImageWidth(wand);
    int x_min = width, x_max = 0;
    PixelWand *pixel_wand = NewPixelWand();

    for (int y = y_min; y <= y_max; y++)
    {
        for (size_t x = 0; x < width; x++)
        {
            if (is_pixel_black(wand, x, y, pixel_wand))
            {
                if ((int)x < x_min) x_min = x;
                if ((int)x > x_max) x_max = x;
            }
        }
    }

    DestroyPixelWand(pixel_wand);
    return (BoundingBox){x_min, y_min, x_max, y_max};
}

// Extract vertical bounding box
BoundingBox extract_bbox_vertical(MagickWand *wand, int x_min, int x_max)
{
    size_t height = MagickGetImageHeight(wand);
    int y_min = height, y_max = 0;
    PixelWand *pixel_wand = NewPixelWand();

    for (int x = x_min; x <= x_max; x++)
    {
        for (size_t y = 0; y < height; y++)
        {
            if (is_pixel_black(wand, x, y, pixel_wand))
            {
                if ((int)y < y_min) y_min = y;
                if ((int)y > y_max) y_max = y;
            }
        }
    }

    DestroyPixelWand(pixel_wand);
    return (BoundingBox){x_min, y_min, x_max, y_max};
}

// Extract zones for vertical layout (grid above/below words)
ExtractedZones
extract_zones_vertical(MagickWand *wand, Zone *zones, int zone_count)
{
    ExtractedZones result = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    size_t height = MagickGetImageHeight(wand);

    if (zone_count >= 2)
    {
        int largest_idx = 0, smallest_idx = 1;
        if (zones[1].density > zones[0].density)
        {
            largest_idx = 1;
            smallest_idx = 0;
        }

        // Increased padding for grid (2.5%), smaller for words (0.8%)
        int grid_padding = (int)(height * 0.025);
        int words_padding = (int)(height * 0.008);
        if (grid_padding < 8) grid_padding = 8;
        if (words_padding < 3) words_padding = 3;

        // Apply padding to zone boundaries
        int grid_y_min = zones[largest_idx].start - grid_padding;
        int grid_y_max = zones[largest_idx].end + grid_padding;
        int words_y_min = zones[smallest_idx].start - words_padding;
        int words_y_max = zones[smallest_idx].end + words_padding;

        // Clamp to image bounds
        if (grid_y_min < 0) grid_y_min = 0;
        if (grid_y_max >= (int)height) grid_y_max = height - 1;
        if (words_y_min < 0) words_y_min = 0;
        if (words_y_max >= (int)height) words_y_max = height - 1;

        result.grid = extract_bbox_horizontal(wand, grid_y_min, grid_y_max);
        result.words = extract_bbox_horizontal(wand, words_y_min, words_y_max);
    }
    else if (zone_count == 1)
    {
        int padding = (int)(height * 0.025);
        if (padding < 8) padding = 8;

        int y_min = zones[0].start - padding;
        int y_max = zones[0].end + padding;
        if (y_min < 0) y_min = 0;
        if (y_max >= (int)height) y_max = height - 1;

        result.grid = extract_bbox_horizontal(wand, y_min, y_max);
    }

    return result;
}

// Extract zones for horizontal layout (grid left/right of words)
ExtractedZones
extract_zones_horizontal(MagickWand *wand, Zone *zones, int zone_count)
{
    ExtractedZones result = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    size_t width = MagickGetImageWidth(wand);

    if (zone_count >= 2)
    {
        int largest_idx = 0, smallest_idx = 1;
        if (zones[1].density > zones[0].density)
        {
            largest_idx = 1;
            smallest_idx = 0;
        }

        // Increased padding for grid (2.5%), smaller for words (0.8%)
        int grid_padding = (int)(width * 0.025);
        int words_padding = (int)(width * 0.008);
        if (grid_padding < 8) grid_padding = 8;
        if (words_padding < 3) words_padding = 3;

        // Apply padding to zone boundaries
        int grid_x_min = zones[largest_idx].start - grid_padding;
        int grid_x_max = zones[largest_idx].end + grid_padding;
        int words_x_min = zones[smallest_idx].start - words_padding;
        int words_x_max = zones[smallest_idx].end + words_padding;

        // Clamp to image bounds
        if (grid_x_min < 0) grid_x_min = 0;
        if (grid_x_max >= (int)width) grid_x_max = width - 1;
        if (words_x_min < 0) words_x_min = 0;
        if (words_x_max >= (int)width) words_x_max = width - 1;

        result.grid = extract_bbox_vertical(wand, grid_x_min, grid_x_max);
        result.words = extract_bbox_vertical(wand, words_x_min, words_x_max);
    }
    else if (zone_count == 1)
    {
        int padding = (int)(width * 0.025);
        if (padding < 8) padding = 8;

        int x_min = zones[0].start - padding;
        int x_max = zones[0].end + padding;
        if (x_min < 0) x_min = 0;
        if (x_max >= (int)width) x_max = width - 1;

        result.grid = extract_bbox_vertical(wand, x_min, x_max);
    }

    return result;
}

// Detect layout and return zones
char detect_layout(
    MagickWand *wand,
    Projections **projs,
    Zone **zones_result,
    int *count
)
{
    *projs = projection(wand);
    size_t height = MagickGetImageHeight(wand);
    size_t width = MagickGetImageWidth(wand);

    int count_h = 0, count_v = 0;
    Zone *zones_h = find_two_main_zones((*projs)->horizontal, height, &count_h);
    Zone *zones_v = find_two_main_zones((*projs)->vertical, width, &count_v);

    char layout = count_v > count_h;  // 1 = horizontal, 0 = vertical
    *zones_result = layout ? zones_v : zones_h;
    *count = layout ? count_v : count_h;

    if (layout) { free(zones_h); }
    else
    {
        free(zones_v);
    }

    return layout;
}

// Main function
ExtractedZones detect_zones(MagickWand *wand)
{
    Projections *projs = NULL;
    Zone *zones = NULL;
    int count = 0;

    char layout = detect_layout(wand, &projs, &zones, &count);
    ExtractedZones result = layout
                                ? extract_zones_horizontal(wand, zones, count)
                                : extract_zones_vertical(wand, zones, count);

    // For horizontal layout, fix both zones if they're incorrectly sized
    if (layout && count >= 2)
    {
        size_t width = MagickGetImageWidth(wand);

        // Check dimensions
        int grid_width = result.grid.x_max - result.grid.x_min;
        int words_width = result.words.x_max - result.words.x_min;

        // FIX 1: If words zone is much narrower than grid (< 60% width), extend
        // it
        if (words_width < grid_width * 0.6)
        {
            // Extend to 95% of image width to avoid catching borders
            result.words.x_max = (int)(width * 0.95);
        }

        // FIX 2: If grid zone starts too far from left edge, extend it left
        if (result.grid.x_min > (int)(width * 0.05))
        {
            result.grid.x_min = (int)(width * 0.02);
        }

        // FIX 3: If grid zone is much narrower than expected, extend it
        if (grid_width < width * 0.35)
        {
            // Extend right edge towards words zone, leaving small gap
            int words_start = result.words.x_min;
            result.grid.x_max = words_start - (int)(width * 0.03);
            if (result.grid.x_max >= (int)width) result.grid.x_max = width - 1;
        }
    }

    free(zones);
    if (projs)
    {
        free(projs->horizontal);
        free(projs->vertical);
        free(projs);
    }

    return result;
}

void DrawZoneBoundries(DrawingWand *draw, BoundingBox *ez, char *color)
{
    PixelWand *stroke_color = NewPixelWand();
    PixelWand *fill_color = NewPixelWand();

    PixelSetColor(fill_color, "white");
    PixelSetColor(stroke_color, color);
    DrawSetStrokeColor(draw, stroke_color);
    DrawSetStrokeWidth(draw, 3);

    PixelSetColor(fill_color, "none");
    DrawSetFillColor(draw, fill_color);
    DrawRectangle(draw, ez->x_min, ez->y_min, ez->x_max, ez->y_max);

    DestroyPixelWand(stroke_color);
    DestroyPixelWand(fill_color);
}
