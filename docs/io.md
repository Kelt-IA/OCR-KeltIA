# I/O Module

Source: `KeltIA/src/io/`  
Headers: `KeltIA/include/io/`

The I/O module provides dataset loaders and conversion utilities that feed data into the neural network training pipeline. It supports MNIST/EMNIST binary files, bitmap folder datasets, and CSV manifests.

---

## Table of Contents

- [bitmap_loader — Bitmap Folder Datasets](#bitmap_loader--bitmap-folder-datasets)
- [image_classifier — Batch Prediction and Organisation](#image_classifier--batch-prediction-and-organisation)
- [mnist_loader — MNIST / EMNIST Format](#mnist_loader--mnist--emnist-format)
- [parse_csv — CSV Parsing](#parse_csv--csv-parsing)
- [path_to_entries — CSV-to-Dataset Conversion](#path_to_entries--csv-to-dataset-conversion)

---

## bitmap_loader — Bitmap Folder Datasets

Header: `include/io/bitmap_loader.h`  
Source: `src/io/bitmap_loader.c`

Loads a training dataset stored as a directory tree where each sub-folder name is the class label (e.g. `dataset/A/`, `dataset/B/`, …).

### Data structures

```c
typedef struct {
    uint8_t **images;   // [num_images][width * height], grayscale
    uint8_t *labels;    // [num_images], integer class index
    size_t num_images;
    size_t width;
    size_t height;
} ImageData;
```

### API

```c
// Load a single image file using MagickWand. Returns a heap-allocated
// grayscale pixel array and writes dimensions to *out_width, *out_height.
uint8_t *load_single_image_magick(const char *filepath,
    size_t *out_width, size_t *out_height);

// Load a single image, resize it to exactly 28×28 while preserving
// the aspect ratio (padding with white). Returns a 784-byte array.
uint8_t *resize_and_pad_to_28x28(const char *filepath);

// Walk a directory tree (A–Z sub-folders) and load all images.
// Returns a heap-allocated ImageData.
ImageData *load_dataset_from_folders(const char *dataset_path);

// Convert an ImageData into a Dataset suitable for train_nn().
// Normalises pixel values to [0, 1] and one-hot encodes labels.
// *out_dataset is allocated by this function; caller must free_dataset().
void images_to_dataset(ImageData *images, Dataset **out_dataset, size_t num_classes);

void free_image_data(ImageData *data);
```

### Dataset folder layout

```
dataset/
├── A/
│   ├── sample1.bmp
│   └── sample2.bmp
├── B/
│   └── sample1.bmp
...
└── Z/
```

Folder names are mapped to class indices alphabetically (A=0, B=1, …, Z=25).

---

## image_classifier — Batch Prediction and Organisation

Header: `include/io/image_classifier.h`  
Source: `src/io/image_classifier.c`

Runs inference on a directory of images and sorts them into labelled output folders — useful for building or auditing datasets.

### Data structures

```c
typedef struct {
    char *filepath;
    uint8_t *pixels;
    size_t width;
    size_t height;
    int predicted_class;
    double confidence;    // output activation of the winning class
} ImagePrediction;
```

### API

```c
// Create sub-folders A–Z under output_dir. Returns 0 on success.
int create_letter_folders(const char *output_dir);

// For each image in input_dir, run inference with nn and copy the
// file into the matching letter folder under output_dir.
int classify_and_organize_images(
    NeuronalNetwork *nn,
    const char *input_dir,
    const char *output_dir,
    size_t image_width,
    size_t image_height
);

// Run inference on a single bitmap file. Returns a heap-allocated
// ImagePrediction; caller must free_prediction().
ImagePrediction *predict_single_image(
    NeuronalNetwork *nn,
    const char *filepath,
    size_t width,
    size_t height
);

// Return 1 if filename has a recognised image extension (.bmp, .png, .jpg, …).
int is_image_file(const char *filename);

void free_prediction(ImagePrediction *pred);
```

---

## mnist_loader — MNIST / EMNIST Format

Header: `include/io/mnist_loader.h`  
Source: `src/io/mnist_loader.c`

Reads the IDX binary format used by MNIST and EMNIST (letters split).

### Data structures

```c
typedef struct {
    uint8_t **images;   // [num_images][image_size], raw pixel values 0–255
    uint8_t *labels;    // [num_images]
    size_t num_images;
} MNISTData;
```

### API

```c
// Load images and labels from their respective IDX files.
// The files may be gzip-compressed; pass the raw .gz path directly.
MNISTData *load_mnist_images(const char *image_path, const char *label_path);

void free_mnist_data(MNISTData *data);

// Convert MNISTData to a Dataset for digit classification (10 classes, labels 0–9).
void mnist_to_dataset(MNISTData *mnist, Dataset **out_dataset);

// Convert MNISTData to a Dataset for EMNIST letters (26 classes, labels 1–26 → 0–25).
void emnist_letters_to_dataset(MNISTData *mnist, Dataset **out_dataset);
```

Both `mnist_to_dataset` and `emnist_letters_to_dataset` normalise pixel values to `[0, 1]` and one-hot encode targets.

---

## parse_csv — CSV Parsing

Header: `include/io/parse_csv.h`  
Source: `src/io/parse_csv.c`

Minimal CSV reader that returns the file as a table of strings.

### Data structures

```c
typedef struct {
    char **labels;     // [num_columns], header row values
    char ***rows;      // [entries][num_columns], data cells
    size_t num_columns;
    size_t entries;    // number of data rows (excluding header)
} CSV;
```

### API

```c
// Read filename, splitting fields on sep (e.g. "," or ";").
// The first row is treated as a header and stored in labels.
// Returns a heap-allocated CSV; caller must free_csv().
CSV *read_csv(const char *filename, const char *sep);

void free_csv(CSV *data);
```

---

## path_to_entries — CSV-to-Dataset Conversion

Header: `include/io/path_to_entries.h`  
Source: `src/io/path_to_entries.c`

Converts a CSV manifest (columns: image path, label) into a `Dataset` for training.

### API

```c
// Build a Dataset from a CSV manifest.
// Each row is expected to have an image path and a class label.
// Images are loaded, resized to target_width × target_height, and normalised.
// base_path is prepended to relative image paths in the CSV.
Dataset *csv_to_dataset(
    CSV *csv_data,
    const char *base_path,
    size_t target_width,
    size_t target_height
);

// Load a single image, resize it to target_width × target_height,
// and return a heap-allocated normalised double array.
// Writes the total pixel count to *pixels_count.
double *read_image_pixels(
    const char *image_path,
    size_t *pixels_count,
    size_t target_width,
    size_t target_height
);
```
