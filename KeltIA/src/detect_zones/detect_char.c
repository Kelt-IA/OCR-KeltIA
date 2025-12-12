#include "../../include/detect_zones/detect_char.h"
#include "../../include/detect_zones/detect_zones.h"

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

                // Add to stack if black pixel found (absolues)
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

    // Extraer la zona correspondiente al carácter
    MagickWand *letter = extract_zone(wand, bbox.x, bbox.y, bbox.w, bbox.h);
    if (letter == NULL) return -1;

    // Obtener dimensiones actuales
    size_t orig_width = MagickGetImageWidth(letter);
    size_t orig_height = MagickGetImageHeight(letter);

    // Calcular el factor de escala para mantener aspect ratio
    // La imagen debe caber dentro de 28x28
    double scale_w = (double)TARGET_SIZE / orig_width;
    double scale_h = (double)TARGET_SIZE / orig_height;
    double scale = fmin(scale_w, scale_h);  // Usar el menor para que quepa

    // Calcular nuevas dimensiones manteniendo aspect ratio
    size_t new_width = (size_t)(orig_width * scale);
    size_t new_height = (size_t)(orig_height * scale);

    // Redimensionar solo si es necesario
    if (new_width != orig_width || new_height != orig_height)
    {
        // MagickResizeImage con LanczosFilter para mejor calidad
        MagickResizeImage(letter, new_width, new_height, LanczosFilter);
    }

    // Configurar el color de fondo blanco para el padding
    PixelWand *bg_color = NewPixelWand();
    PixelSetColor(bg_color, "white");
    MagickSetImageBackgroundColor(letter, bg_color);
    DestroyPixelWand(bg_color);

    // Configurar gravedad para centrar la imagen
    MagickSetGravity(letter, CenterGravity);

    // Calcular offset para centrar la imagen
    ssize_t offset_x = (TARGET_SIZE - new_width) / 2;
    ssize_t offset_y = (TARGET_SIZE - new_height) / 2;

    // Extender la imagen a 28x28 con padding blanco centrado
    MagickExtentImage(letter, TARGET_SIZE, TARGET_SIZE, -offset_x, -offset_y);

    // Guardar en formato BMP
    MagickBooleanType status = MagickWriteImage(letter, path);

    DestroyMagickWand(letter);

    return (status == MagickTrue) ? 0 : -1;
}
