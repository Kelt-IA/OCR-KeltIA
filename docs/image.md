# Image Processing Module

Source: `KeltIA/src/image/`  
Headers: `KeltIA/include/image/`

All image operations are built on [ImageMagick's MagickWand C API](https://imagemagick.org/script/magick-wand.php). Every function that returns a `MagickWand *` allocates a new wand; the caller is responsible for destroying it with `DestroyMagickWand()`.

---

## Table of Contents

- [treatment — Master Pipeline](#treatment--master-pipeline)
- [grayscale — Grayscale and Binarization](#grayscale--grayscale-and-binarization)
- [rotation — Manual Rotation](#rotation--manual-rotation)
- [autorotation — Automatic Deskewing](#autorotation--automatic-deskewing)
- [removenoise — Noise Removal](#removenoise--noise-removal)
- [show_result — Result Overlay](#show_result--result-overlay)

---

## treatment — Master Pipeline

Header: `include/image/treatment.h`  
Source: `src/image/treatment.c`

The treatment module is the single entry point for preprocessing. It coordinates all other sub-modules and produces two versions of the image: one optimised for neural network input and one suitable for display in the GUI.

```c
typedef struct {
    char *nn_image_path;   // path to the image prepared for the CNN
    char *ui_image_path;   // path to the image prepared for display
} ProcessedImages;

// Apply user-selected preprocessing steps and write both output images to disk.
// Returns a heap-allocated ProcessedImages; caller must free the struct and its strings.
ProcessedImages *process_image(
    const char *input_path,
    int apply_grayscale,
    int apply_black_and_white,
    int apply_noise,
    int auto_rotation,
    double manual_angle      // ignored when auto_rotation != 0
);

// Apply all treatments unconditionally (grayscale → binarize → denoise → auto-rotate).
MagickWand *apply_all_treatments(MagickWand *wand, int auto_rotation);

// Apply only the treatments selected by the boolean flags.
MagickWand *apply_selected_treatments(
    MagickWand *wand,
    int apply_grayscale,
    int apply_black_and_white,
    int apply_noise,
    int auto_rotation,
    double manual_angle
);
```

Typical call for the full pipeline:

```c
ProcessedImages *imgs = process_image("puzzle.png", 1, 1, 1, 1, 0.0);
// imgs->nn_image_path → ready for the CNN
// imgs->ui_image_path → ready to display
free(imgs->nn_image_path);
free(imgs->ui_image_path);
free(imgs);
```

---

## grayscale — Grayscale and Binarization

Header: `include/image/grayscale.h`  
Source: `src/image/grayscale.c`

```c
// Compute the average luminance of the image (used for adaptive thresholding).
double compute_average_gray(MagickWand *wand);

// Convert to grayscale and apply Otsu-style thresholding; write result to output_path.
MagickBooleanType binarize_image(const char *input_path, const char *output_path);

// Load an image from input_path and return a binarized in-memory MagickWand.
MagickWand *binarize_image_wand(const char *input_path);

// Load an image and return a grayscale-only in-memory MagickWand.
MagickWand *grayscale_image_wand(const char *input_path);
```

`binarize_image_wand` is the preferred in-memory variant used by the pipeline. It returns a new `MagickWand`; destroy it when done.

---

## rotation — Manual Rotation

Header: `include/image/rotation.h`  
Source: `src/image/rotation.c`

```c
// Rotate wand by angle degrees (clockwise). Returns a new MagickWand.
MagickWand *rotate_image(const MagickWand *input_wand, double angle);
```

The returned wand is independent of `input_wand`.

---

## autorotation — Automatic Deskewing

Header: `include/image/autorotation.h`  
Source: `src/image/autorotation.c`

Automatic deskewing analyses the image to estimate and correct its skew angle so that text lines become horizontal.

```c
// Detect skew angle using projection analysis and correct it.
// Returns a new MagickWand with the deskewed image.
MagickWand *auto_rotate_image(MagickWand *input_wand);

// Brute-force variant: sweep angles in steps of angle_step degrees,
// choosing the angle that maximises horizontal projection sharpness.
MagickWand *auto_rotate_image_bruteforce(MagickWand *input_wand, double angle_step);
```

`auto_rotate_image` is faster; `auto_rotate_image_bruteforce` is more accurate but slower. The pipeline uses `auto_rotate_image` by default.

---

## removenoise — Noise Removal

Header: `include/image/removenoise.h`  
Source: `src/image/removenoise.c`

```c
// Apply a fixed Gaussian-based denoising pass.
MagickWand *remove_noise(MagickWand *wand);

// Apply adaptive denoising; strength ∈ [0.0, 1.0].
MagickWand *remove_noise_adaptive(MagickWand *wand, double strength);

// Remove isolated salt-and-pepper pixels from a binary image.
MagickWand *clean_binary_image(MagickWand *wand);

// Load an image from path into a new MagickWand.
MagickWand *read_image(const char *path);

// Write wand to path. Returns 1 on success.
int write_image(MagickWand *wand, const char *path);

// Compute pixel luminance standard deviation (used to pick denoising strength).
double compute_gray_stddev(MagickWand *wand);

// Suggest a denoising radius given an image's standard deviation.
double auto_radius(double stddev);

// Estimate line density (proxy for how much detail to preserve).
double detect_fine_lines(MagickWand *wand);

// Choose denoising strength automatically from image statistics.
double auto_strength(MagickWand *wand);
```

For pipeline use, `remove_noise_adaptive` with `auto_strength(wand)` gives the best results across varied inputs.

---

## show_result — Result Overlay

Header: `include/image/show_result.h`  
Source: `src/image/show_result.c`

These functions draw the solved crossword result back onto the original image for display in the GUI.

```c
// Draw all found word highlights on wand. Words is a null-terminated array of strings.
void show_result(
    CharBBox **grid,
    char *char_grid,
    int height,
    int width,
    char **words,
    MagickWand *wand
);

// Draw a coloured bounding box around a single word's path through the grid.
void draw_word_box(
    CharBBox **grid,
    WordPos *pos,
    MagickWand *wand,
    int width,
    int height
);

// Draw a line between two pixel coordinates (used to connect first and last letter).
void draw_line_between_letters(
    MagickWand *image_wand,
    double x1, double y1,
    double x2, double y2,
    double thickness
);
```

`show_result` is the high-level call; it internally calls `draw_word_box` for each word returned by the solver.
