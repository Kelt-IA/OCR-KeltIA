#include "../../include/detect_zones/detect_letters.h"
#include <MagickWand/MagickWand.h>
#include <stdlib.h>

// Helper function to filter valid character components
static int is_valid_character(
    CCObjectInfo *object,
    int min_area,
    int max_area,
    float min_aspect,
    float max_aspect
)
{
    int width = (int)(object->bounding_box.width);
    int height = (int)(object->bounding_box.height);
    int area = width * height;
    float aspect_ratio = (height > 0) ? (float)width / height : 0.0f;

    // Filter by area and aspect ratio to remove noise
    if (area < min_area || area > max_area) { return 0; }

    if (aspect_ratio < min_aspect || aspect_ratio > max_aspect) { return 0; }

    // Filter very small components (likely noise)
    if (width < 5 || height < 5) { return 0; }

    return 1;
}

// Check if bbox1 is completely inside bbox2
static int is_bbox_inside(CharBBox *bbox1, CharBBox *bbox2)
{
    return (
        bbox1->x >= bbox2->x && bbox1->y >= bbox2->y &&
        bbox1->x + bbox1->w <= bbox2->x + bbox2->w &&
        bbox1->y + bbox1->h <= bbox2->y + bbox2->h
    );
}

// Remove nested bounding boxes (inner components of letters like O, A, B, D)
static int filter_nested_bboxes(CharBBox *characters, int char_count)
{
    if (char_count <= 1) return char_count;

    // Create array to mark which bboxes to keep
    char *keep = malloc(char_count * sizeof(char));
    if (keep == NULL) return char_count;  // If malloc fails, return original

    for (int i = 0; i < char_count; i++)
    {
        keep[i] = 1;  // Assume we keep it initially
    }

    // Mark nested boxes for removal
    for (int i = 0; i < char_count; i++)
    {
        if (!keep[i]) continue;

        for (int j = 0; j < char_count; j++)
        {
            if (i == j || !keep[j]) continue;

            // If character i is completely inside character j
            if (is_bbox_inside(&characters[i], &characters[j]))
            {
                // Keep the larger one (j), remove the smaller one (i)
                keep[i] = 0;
                break;
            }
        }
    }

    // Compact array - remove unmarked characters
    int write_index = 0;
    for (int read_index = 0; read_index < char_count; read_index++)
    {
        if (keep[read_index])
        {
            if (write_index != read_index)
            {
                characters[write_index] = characters[read_index];
            }
            write_index++;
        }
    }

    free(keep);
    return write_index;  // Return new count
}

// Main character detection function
CharBBox *detect_letters(
    MagickWand *wand,
    int x_offset,
    int y_offset,
    int width,
    int height,
    int *char_count
)
{
    MagickWand *cropped_wand = NULL;
    CCObjectInfo *objects = NULL;
    CharBBox *results = NULL;
    size_t num_objects = 0;
    int valid_count = 0;

    *char_count = 0;

    // Step 1: Crop the region of interest
    cropped_wand = CloneMagickWand(wand);
    if (cropped_wand == NULL) { return NULL; }

    MagickCropImage(cropped_wand, width, height, x_offset, y_offset);
    MagickResetImagePage(cropped_wand, "");

    // Step 2: Ensure the image is properly binarized
    // Use a simple threshold (50% of quantum range)
    MagickThresholdImage(cropped_wand, QuantumRange * 0.5);

    // Step 3: Run connected components analysis
    // Using 4-connectivity to avoid merging touching letters
    MagickBooleanType status = MagickConnectedComponentsImage(
        cropped_wand,
        4,  // 4-way connectivity (only horizontal/vertical, no diagonals)
        &objects
    );

    if (status == MagickFalse || objects == NULL)
    {
        DestroyMagickWand(cropped_wand);
        return NULL;
    }

    // Step 4: Get number of objects
    num_objects = (size_t)MagickGetImageColors(cropped_wand);

    // Step 5: Calculate adaptive thresholds based on region size
    int total_area = width * height;
    int min_char_area = (int)(total_area * 0.00005);  // 0.005% of total area
    int max_char_area = (int)(total_area * 0.1);      // 10% of total area
    float min_aspect = 0.1f;                          // Very thin characters
    float max_aspect = 5.0f;                          // Very wide characters

    // Step 6: Count valid characters first
    for (size_t i = 1; i < num_objects; i++)
    {  // Skip background (id=0)
        if (is_valid_character(
                &objects[i], min_char_area, max_char_area, min_aspect,
                max_aspect
            ))
        {
            valid_count++;
        }
    }

    // Step 7: Allocate results array
    if (valid_count == 0)
    {
        objects = (CCObjectInfo *)RelinquishMagickMemory(objects);
        DestroyMagickWand(cropped_wand);
        return NULL;
    }

    results = (CharBBox *)malloc(valid_count * sizeof(CharBBox));
    if (results == NULL)
    {
        objects = (CCObjectInfo *)RelinquishMagickMemory(objects);
        DestroyMagickWand(cropped_wand);
        return NULL;
    }

    // Step 8: Extract bounding boxes for valid characters
    int result_index = 0;
    for (size_t i = 1; i < num_objects && result_index < valid_count; i++)
    {
        if (is_valid_character(
                &objects[i], min_char_area, max_char_area, min_aspect,
                max_aspect
            ))
        {
            // Convert coordinates back to original image space
            results[result_index].x =
                (int)(objects[i].bounding_box.x) + x_offset;
            results[result_index].y =
                (int)(objects[i].bounding_box.y) + y_offset;
            results[result_index].w = (int)(objects[i].bounding_box.width);
            results[result_index].h = (int)(objects[i].bounding_box.height);
            result_index++;
        }
    }

    *char_count = valid_count;

    // Step 9: Filter nested bounding boxes (removes inner holes of O, A, B, D,
    // etc.)
    *char_count = filter_nested_bboxes(results, *char_count);

    // Step 10: Cleanup
    objects = (CCObjectInfo *)RelinquishMagickMemory(objects);
    DestroyMagickWand(cropped_wand);

    return results;
}
