#include "../../include/detect_zones/detect_zones.h"

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

// Find two main zones from a projection
Zone *find_two_main_zones(int *proj, int size, int *count)
{
    Zone *zones = malloc(sizeof(Zone) * 2);
    *count = 0;

    int threshold = 1;
    int in_zone = 0;
    int zone_start = 0;
    int zone_density = 0;

    for (int i = 0; i < size; i++)
    {
        if (proj[i] > threshold)
        {
            if (!in_zone)
            {
                zone_start = i;
                zone_density = 0;
                in_zone = 1;
            }
            zone_density += proj[i];
        }
        else
        {
            if (in_zone && *count < 2)
            {
                zones[*count].start = zone_start;
                zones[*count].end = i - 1;
                zones[*count].density = zone_density;
                (*count)++;
                in_zone = 0;
            }
        }
    }

    // Handle last zone if exists
    if (in_zone && *count < 2)
    {
        zones[*count].start = zone_start;
        zones[*count].end = size - 1;
        zones[*count].density = zone_density;
        (*count)++;
    }

    return zones;
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

    if (zone_count >= 2)
    {
        int largest_idx = 0, smallest_idx = 1;
        if (zones[1].density > zones[0].density)
        {
            largest_idx = 1;
            smallest_idx = 0;
        }

        result.grid = extract_bbox_horizontal(
            wand, zones[largest_idx].start, zones[largest_idx].end
        );
        result.words = extract_bbox_horizontal(
            wand, zones[smallest_idx].start, zones[smallest_idx].end
        );
    }
    else if (zone_count == 1)
    {
        result.grid =
            extract_bbox_horizontal(wand, zones[0].start, zones[0].end);
    }

    return result;
}

// Extract zones for horizontal layout (grid left/right of words)
ExtractedZones
extract_zones_horizontal(MagickWand *wand, Zone *zones, int zone_count)
{
    ExtractedZones result = {{0, 0, 0, 0}, {0, 0, 0, 0}};

    if (zone_count >= 2)
    {
        int largest_idx = 0, smallest_idx = 1;
        if (zones[1].density > zones[0].density)
        {
            largest_idx = 1;
            smallest_idx = 0;
        }

        result.grid = extract_bbox_vertical(
            wand, zones[largest_idx].start, zones[largest_idx].end
        );
        result.words = extract_bbox_vertical(
            wand, zones[smallest_idx].start, zones[smallest_idx].end
        );
    }
    else if (zone_count == 1)
    {
        result.grid = extract_bbox_vertical(wand, zones[0].start, zones[0].end);
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
    else { free(zones_v); }

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

    if (layout)
    {
        size_t width = MagickGetImageWidth(wand);
        int right_margin = (int)(width * 0.05);  // 5% margin
        result.words.x_max = width - right_margin;
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
