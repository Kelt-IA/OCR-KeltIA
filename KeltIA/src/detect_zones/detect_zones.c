#include "../../include/detect_zones/detect_zones.h"

#include <stdlib.h>

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

// Find ALL zones - MODIFIED FOR BETTER DETECTION
Zone *find_all_zones(int *proj, int size, int *count)
{
    Zone *zones = malloc(sizeof(Zone) * 20);
    *count = 0;

    int max_proj = 0;
    long long total_proj = 0;
    for (int i = 0; i < size; i++)
    {
        if (proj[i] > max_proj) max_proj = proj[i];
        total_proj += proj[i];
    }

    int avg_proj = total_proj / size;
    int threshold = avg_proj / 5; // Plus sensible: 20%
    if (threshold < 2) threshold = 2; // Plus bas
    if (threshold > max_proj / 10) threshold = max_proj / 10;

    int in_zone = 0;
    int zone_start = 0;
    int zone_density = 0;
    int gap = 0;

    int max_gap = size / 50;
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
                in_zone = 1;
            }
            zone_density += proj[i];
            gap = 0;
        }
        else
        {
            if (in_zone)
            {
                gap++;
                if (gap > max_gap)
                {
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

    if (in_zone && *count < 20)
    {
        zones[*count].start = zone_start;
        zones[*count].end = size - 1;
        zones[*count].density = zone_density;
        (*count)++;
    }

    // Filter SMALL zones mais plus permissif
    if (*count > 2)
    {
        int min_size = size / 60; // 1.7% au lieu de 3%
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

    return zones;
}

// Find two main zones - NOW HANDLES 3-ZONE CASE
Zone *find_two_main_zones(int *proj, int size, int *count)
{
    int all_count = 0;
    Zone *all_zones = find_all_zones(proj, size, &all_count);

    if (all_count <= 2)
    {
        *count = all_count;
        return all_zones;
    }

    // Sort by density descending
    for (int i = 0; i < all_count - 1; i++)
    {
        for (int j = i + 1; j < all_count; j++)
        {
            if (all_zones[j].density > all_zones[i].density)
            {
                Zone temp = all_zones[i];
                all_zones[i] = all_zones[j];
                all_zones[j] = temp;
            }
        }
    }

    // SPECIAL CASE: 3 zones - check if it's a sandwich (grid-words-grid)
    if (all_count == 3)
    {
        // Reorder by position temporarily
        Zone temp_zones[3];
        for (int i = 0; i < 3; i++) temp_zones[i] = all_zones[i];
        
        for (int i = 0; i < 2; i++)
        {
            for (int j = i + 1; j < 3; j++)
            {
                if (temp_zones[j].start < temp_zones[i].start)
                {
                    Zone temp = temp_zones[i];
                    temp_zones[i] = temp_zones[j];
                    temp_zones[j] = temp;
                }
            }
        }
        
        // Check if middle zone is much smaller (words between grids)
        int top_size = temp_zones[0].end - temp_zones[0].start;
        int mid_size = temp_zones[1].end - temp_zones[1].start;
        int bot_size = temp_zones[2].end - temp_zones[2].start;
        
        int avg_outer = (top_size + bot_size) / 2;
        
        // If middle is < 40% of outer average → sandwich layout
        if (mid_size < avg_outer * 0.4)
        {
            // Return the two outer zones (top and middle, NOT top and bottom)
            // We want ONE grid zone and the words zone
            // Choose the denser outer zone as grid
            Zone *result = malloc(sizeof(Zone) * 2);
            
            if (temp_zones[0].density > temp_zones[2].density)
            {
                result[0] = temp_zones[0]; // Top grid
                result[1] = temp_zones[1]; // Middle words
            }
            else
            {
                result[0] = temp_zones[1]; // Middle words
                result[1] = temp_zones[2]; // Bottom grid
            }
            
            // Reorder by position
            if (result[0].start > result[1].start)
            {
                Zone temp = result[0];
                result[0] = result[1];
                result[1] = temp;
            }
            
            *count = 2;
            free(all_zones);
            return result;
        }
    }

    // Normal case: take top 2 by density
    Zone *result = malloc(sizeof(Zone) * 2);
    result[0] = all_zones[0];
    result[1] = all_zones[1];
    *count = 2;

    // Reorder by position
    if (result[0].start > result[1].start)
    {
        Zone temp = result[0];
        result[0] = result[1];
        result[1] = temp;
    }

    free(all_zones);
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

// Extract zones for vertical layout
ExtractedZones extract_zones_vertical(MagickWand *wand, Zone *zones, int zone_count)
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
        
        int grid_padding = (int)(height * 0.015); // 1.5%
        int words_padding = (int)(height * 0.005); // 0.5%
        if (grid_padding < 4) grid_padding = 4;
        if (words_padding < 2) words_padding = 2;
        
        int grid_y_min = zones[largest_idx].start - grid_padding;
        int grid_y_max = zones[largest_idx].end + grid_padding;
        int words_y_min = zones[smallest_idx].start - words_padding;
        int words_y_max = zones[smallest_idx].end + words_padding;
        
        // Calculate midpoint in the GAP
        int gap = zones[1].start - zones[0].end;
        int midpoint = zones[0].end + gap / 2;
        
        // Top zone stops at midpoint
        if (zones[largest_idx].start < midpoint && grid_y_max > midpoint)
            grid_y_max = midpoint - 1;
        if (zones[smallest_idx].start < midpoint && words_y_max > midpoint)
            words_y_max = midpoint - 1;
            
        // Bottom zone starts at midpoint
        if (zones[largest_idx].start >= midpoint && grid_y_min < midpoint)
            grid_y_min = midpoint + 1;
        if (zones[smallest_idx].start >= midpoint && words_y_min < midpoint)
            words_y_min = midpoint + 1;
        
        if (grid_y_min < 0) grid_y_min = 0;
        if (grid_y_max >= (int)height) grid_y_max = height - 1;
        if (words_y_min < 0) words_y_min = 0;
        if (words_y_max >= (int)height) words_y_max = height - 1;
        
        result.grid = extract_bbox_horizontal(wand, grid_y_min, grid_y_max);
        result.words = extract_bbox_horizontal(wand, words_y_min, words_y_max);
    }
    else if (zone_count == 1)
    {
        int padding = (int)(height * 0.015);
        if (padding < 4) padding = 4;
        
        int y_min = zones[0].start - padding;
        int y_max = zones[0].end + padding;
        if (y_min < 0) y_min = 0;
        if (y_max >= (int)height) y_max = height - 1;
        
        result.grid = extract_bbox_horizontal(wand, y_min, y_max);
    }
    
    return result;
}

// Extract zones for horizontal layout
ExtractedZones extract_zones_horizontal(MagickWand *wand, Zone *zones, int zone_count)
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
        
        int grid_padding = (int)(width * 0.015);
        int words_padding = (int)(width * 0.005);
        if (grid_padding < 4) grid_padding = 4;
        if (words_padding < 2) words_padding = 2;
        
        int grid_x_min = zones[largest_idx].start - grid_padding;
        int grid_x_max = zones[largest_idx].end + grid_padding;
        int words_x_min = zones[smallest_idx].start - words_padding;
        int words_x_max = zones[smallest_idx].end + words_padding;
        
        int gap = zones[1].start - zones[0].end;
        int midpoint = zones[0].end + gap / 2;
        
        if (zones[largest_idx].start < midpoint && grid_x_max > midpoint)
            grid_x_max = midpoint - 1;
        if (zones[smallest_idx].start < midpoint && words_x_max > midpoint)
            words_x_max = midpoint - 1;
            
        if (zones[largest_idx].start >= midpoint && grid_x_min < midpoint)
            grid_x_min = midpoint + 1;
        if (zones[smallest_idx].start >= midpoint && words_x_min < midpoint)
            words_x_min = midpoint + 1;
        
        if (grid_x_min < 0) grid_x_min = 0;
        if (grid_x_max >= (int)width) grid_x_max = width - 1;
        if (words_x_min < 0) words_x_min = 0;
        if (words_x_max >= (int)width) words_x_max = width - 1;
        
        result.grid = extract_bbox_vertical(wand, grid_x_min, grid_x_max);
        result.words = extract_bbox_vertical(wand, words_x_min, words_x_max);
    }
    else if (zone_count == 1)
    {
        int padding = (int)(width * 0.015);
        if (padding < 4) padding = 4;
        
        int x_min = zones[0].start - padding;
        int x_max = zones[0].end + padding;
        if (x_min < 0) x_min = 0;
        if (x_max >= (int)width) x_max = width - 1;
        
        result.grid = extract_bbox_vertical(wand, x_min, x_max);
    }
    
    return result;
}

// Detect layout and return zones
char detect_layout(MagickWand *wand, Projections **projs, Zone **zones_result, int *count)
{
    *projs = projection(wand);
    size_t height = MagickGetImageHeight(wand);
    size_t width = MagickGetImageWidth(wand);

    int count_h = 0, count_v = 0;
    Zone *zones_h = find_two_main_zones((*projs)->horizontal, height, &count_h);
    Zone *zones_v = find_two_main_zones((*projs)->vertical, width, &count_v);

    char layout = count_v > count_h;
    *zones_result = layout ? zones_v : zones_h;
    *count = layout ? count_v : count_h;

    if (layout)
        free(zones_h);
    else
        free(zones_v);

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
