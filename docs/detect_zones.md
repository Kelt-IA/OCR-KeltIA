# Zone and Character Detection Module

Source: `KeltIA/src/detect_zones/`  
Headers: `KeltIA/include/detect_zones/`

This module is responsible for two sequential tasks:

1. **Zone detection** (`detect_zones`) — locates the two main regions of a crossword image: the letter grid and the word list.
2. **Character segmentation** (`detect_char`) — within a detected zone, finds the bounding box of each individual character using flood fill.

Both modules operate on `MagickWand` objects from ImageMagick.

---

## Table of Contents

- [detect_zones — Zone Detection](#detect_zones--zone-detection)
- [detect_char — Character Segmentation](#detect_char--character-segmentation)

---

## detect_zones — Zone Detection

Header: `include/detect_zones/detect_zones.h`  
Source: `src/detect_zones/detect_zones.c`

### Key data structures

```c
// Raw horizontal and vertical projection histograms.
typedef struct {
    int *horizontal;
    int *vertical;
} Projections;

// Axis-aligned bounding box.
typedef struct {
    int x_min, y_min, x_max, y_max;
} BoundingBox;

// Pair of bounding boxes: one for the grid, one for the word list.
typedef struct {
    BoundingBox grid;
    BoundingBox words;
} ExtractedZones;

// A contiguous non-empty run along a projection axis.
typedef struct {
    int start, end;
    int density;
} Zone;
```

### API

```c
// Return 1 if the pixel at (x, y) is considered black.
char is_pixel_black(MagickWand *wand, size_t x, size_t y, PixelWand *pixel_wand);

// Compute horizontal and vertical projection histograms.
// Returns a heap-allocated Projections; caller must free both arrays and the struct.
Projections *projection(MagickWand *wand);

// Given a 1D projection array, find the two largest zones separated by whitespace.
// Writes the count to *count and returns a heap-allocated Zone array.
Zone *find_two_main_zones(int *proj, int size, int *count);

// Find the tight bounding box of content between two horizontal lines.
BoundingBox extract_bbox_horizontal(MagickWand *wand, int y_min, int y_max);

// Find the tight bounding box of content between two vertical lines.
BoundingBox extract_bbox_vertical(MagickWand *wand, int x_min, int x_max);

// High-level helpers that take detected zones and return ExtractedZones.
ExtractedZones extract_zones_vertical(MagickWand *wand, Zone *zones, int zone_count);
ExtractedZones extract_zones_horizontal(MagickWand *wand, Zone *zones, int zone_count);

// Detect whether the layout is horizontal (grid top, words bottom)
// or vertical (grid left, words right), then pick the appropriate extractor.
// Returns 'H' or 'V'. Also exposes the computed projections and zones via out params.
char detect_layout(MagickWand *wand, Projections **projs, Zone **zone, int *count);

// Main entry point: detect both zones in one call.
ExtractedZones detect_zones(MagickWand *wand);

// Draw a coloured rectangle around a BoundingBox onto a DrawingWand.
void DrawZoneBoundries(DrawingWand *draw, BoundingBox *ez, char *color);
```

### How it works

1. `projection()` computes how many black pixels are in each row and column.
2. `find_two_main_zones()` scans the projection for the two largest dense runs — these correspond to the grid and word-list blocks.
3. `detect_layout()` decides whether they are stacked vertically or side by side.
4. The appropriate `extract_zones_*` function refines each run into a tight `BoundingBox`.

---

## detect_char — Character Segmentation

Header: `include/detect_zones/detect_char.h`  
Source: `src/detect_zones/detect_char.c`

### Key data structures

```c
// Bounding box of a single detected character (pixel coordinates).
typedef struct {
    int x, y;   // top-left corner
    int w, h;   // width and height
} CharBBox;

typedef struct {
    int x, y;
} Point;
```

### API

```c
// Iterative flood fill that finds the extents of a connected black component
// starting at (start_x, start_y). Updates *x_min, *x_max, *y_min, *y_max.
void flood_fill(
    MagickWand *wand,
    int start_x, int start_y,
    char *visited,
    int width, int height,
    int x_offset, int y_offset,
    int *x_min, int *x_max, int *y_min, int *y_max
);

// Detect all characters in the rectangular region of wand defined by
// (x_offset, y_offset, width, height). Returns a heap-allocated array
// of CharBBox and writes the count to *char_count.
CharBBox *detect_characters(
    MagickWand *wand,
    int x_offset, int y_offset,
    int width, int height,
    int *char_count
);

// Crop a rectangular region from wand and return it as a new MagickWand.
MagickWand *extract_zone(MagickWand *wand, int x, int y, int width, int height);

// Draw coloured rectangles around all detected character bounding boxes.
void DrawLetterBoundries(
    MagickWand *wand,
    DrawingWand *draw,
    CharBBox *grid_characters,
    int grid_chars,
    char *color
);

// Crop the character at bbox from wand, resize to 28×28, and save as BMP.
// Returns 0 on success.
int save_charbbox_as_bitmap(MagickWand *wand, CharBBox bbox, const char *path);

// Crop the character at bbox from wand, resize to 28×28, and return
// a heap-allocated double array of normalised pixel values ∈ [0, 1].
// Length is always 28×28 = 784. Caller must free.
double *charbbox_to_cnn_input(MagickWand *wand, CharBBox bbox);
```

### How it works

`detect_characters` scans every pixel in the given region. When it finds an unvisited black pixel it calls `flood_fill` to collect the full connected component and record its bounding box. Small components (noise) are discarded by a minimum-size threshold. The resulting `CharBBox` array is then sorted left-to-right, top-to-bottom to match reading order.

`charbbox_to_cnn_input` is the bridge between detection and classification: it extracts a character, normalises it to 28×28 pixels, and produces the flat `double` input vector expected by the CNN.
