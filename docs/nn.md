# Neural Network Module

Source: `KeltIA/src/nn/`  
Headers: `KeltIA/include/nn/`

This module is a complete neural network engine written in C with no external ML dependencies. It supports dense (fully-connected) layers and convolutional layers, mini-batch SGD with backpropagation, multiple activation functions, model persistence, and background training with UI callbacks.

---

## Table of Contents

- [Data Structures](#data-structures)
- [Activation Functions](#activation-functions)
- [Layer API](#layer-api)
- [Convolution API](#convolution-api)
- [Network API](#network-api)
- [Backpropagation](#backpropagation)
- [Training](#training)
- [Accuracy Metrics](#accuracy-metrics)
- [Model I/O](#model-io)
- [UI Callbacks and Background Training](#ui-callbacks-and-background-training)
- [Grid Builder](#grid-builder)
- [Error Codes](#error-codes)
- [Configuration](#configuration)

---

## Data Structures

### `Layer` (`include/nn/layers.h`)

A single fully-connected layer.

```c
typedef struct {
    size_t n_inputs;
    size_t n_neurons;
    double *weights;          // [n_neurons × n_inputs] row-major
    double *bias;             // [n_neurons]
    double *z;                // pre-activation values [n_neurons]
    double *output;           // post-activation values [n_neurons]
    ActivationType activation_type;
    ActivationFunction activation_fn;
    DerivativeFunction derivative_fn;
} Layer;
```

Access weight `(i, j)` with the macro `WEIGHT(layer, i, j)`.

### `ConvLayer` (`include/nn/convolution.h`)

A 2D convolutional layer.

```c
typedef struct {
    size_t input_channels, input_height, input_width;
    size_t n_filters, kernel_height, kernel_width;
    size_t stride, padding;
    size_t output_height, output_width;
    double *kernels;      // [n_filters][input_channels][kH][kW]
    double *bias;         // [n_filters]
    double *output;       // [n_filters][output_height][output_width]
    double *input_cache;  // saved input for backprop
    double *col_buffer;   // im2col workspace
} ConvLayer;
```

### `NeuronalNetwork` (`include/nn/network.h`)

Holds a sequential CNN + dense network.

```c
typedef struct {
    size_t n_conv_layers;
    ConvLayer *conv_layers;
    double *flattened;        // buffer between conv and dense
    size_t flattened_size;
    size_t n_layers;
    Layer *layers;            // dense layers
    double learning_rate;
} NeuronalNetwork;
```

### `Dataset` (`include/nn/accuracy_metrics.h`)

Training/evaluation data container.

```c
typedef struct {
    double **inputs;    // [num_samples][input_size]
    double **targets;   // [num_samples][output_size]
    int num_samples;
    size_t input_size;
    size_t output_size;
} Dataset;
```

---

## Activation Functions

Defined in `include/nn/activation.h`, implemented in `src/nn/activation.c`.

| `ActivationType` | Function | Derivative |
|---|---|---|
| `ACTIVATION_SIGMOID` | `sigmoid(z)` | `sigmoid_derivative(z, a)` |
| `ACTIVATION_LEAKY_RELU` | `leaky_relu(z)` | `leaky_relu_derivative(z, a)` |
| `ACTIVATION_SOFTMAX` | `softmax_activation(z, out, size)` | `softmax_derivative(z, a)` |
| `ACTIVATION_STEP` | `step(x)` | — |

Convert an integer (e.g. loaded from disk) to an `ActivationType` with `int_to_activation(int value)`.

---

## Layer API

Defined in `include/nn/layers.h`, implemented in `src/nn/layers.c`.

```c
// Allocate and initialise a layer. Returns 0 on success.
int create_layer(Layer *layer, size_t n_inputs, size_t n_neurons, ActivationType activation);

// Run forward pass. Writes results to layer->z and layer->output.
void forward_layer(Layer *layer, double *input);

// Apply softmax in-place (call after forward_layer for the output layer).
void softmax(Layer *layer);

// Assign a new activation type and its function pointers.
void set_activation(Layer *layer, ActivationType type);

// Randomise weights with He/Xavier initialisation.
void init_weights(Layer *layer);

// Overwrite weights/biases from an existing array.
void load_weights(Layer *layer, double *weights);
void load_biases(Layer *layer, double *biases);

// Load weights/biases from an open binary file handle.
ErrorCode load_weights_from_fs(FILE *f, Layer *layer);
ErrorCode load_biases_from_fs(FILE *f, Layer *layer);

void free_layer(Layer *layer);
```

---

## Convolution API

Defined in `include/nn/convolution.h`, implemented in `src/nn/convolution.c`.

```c
// Allocate a conv layer. Returns 0 on success.
int create_conv_layer(ConvLayer *conv,
    size_t input_channels, size_t input_height, size_t input_width,
    size_t n_filters, size_t kernel_height, size_t kernel_width,
    size_t stride, size_t padding);

// Randomise kernel weights.
void init_conv_weights(ConvLayer *conv);

// Forward pass (im2col + matrix multiply). Writes to conv->output.
void forward_conv_layer(ConvLayer *conv, const double *input);

// Apply an activation function element-wise to the conv output.
void apply_activation_conv(ConvLayer *conv, double (*activation_fn)(double));

// In-place im2col transformation (used internally by forward_conv_layer).
void im2col(const double *input, size_t channels, size_t height, size_t width,
    size_t kernel_h, size_t kernel_w, size_t stride, size_t padding,
    double *col_buffer);

void free_conv_layer(ConvLayer *conv);
```

---

## Network API

Defined in `include/nn/network.h`, implemented in `src/nn/network.c`.

### Create a dense-only network

```c
ErrorCode create_nn(
    size_t n_inputs,
    size_t n_layers,
    size_t neurons_per_layer[n_layers],
    NeuronalNetwork *out_nn
);
```

All hidden layers use `ACTIVATION_SIGMOID`; the output layer uses `ACTIVATION_SOFTMAX`.

### Create a CNN (conv + dense)

```c
ErrorCode create_cnn(
    size_t n_conv_layers,
    ConvLayer *conv_configs,      // pre-configured ConvLayer structs
    size_t n_dense_layers,
    size_t *dense_neurons,
    ActivationType *dense_activations,
    NeuronalNetwork *out_nn
);
```

### Inference

```c
// Run a full forward pass. Writes the output of the last layer to `output`.
void compute_nn(NeuronalNetwork *nn, double *input, double *output);
```

### Cleanup

```c
void free_nn(NeuronalNetwork *nn);
```

---

## Backpropagation

Defined in `include/nn/back-propagation.h`, implemented in `src/nn/back-propagation.c` and `src/nn/back-propagation-convolutional.c`.

These functions are called by the training loop and are not typically called directly.

```c
// Compute delta for the output layer (MSE + activation derivative).
void delta_output(Layer *last_layer, double *expected,
    double *out_delta, size_t out_delta_size);

// Compute delta for a hidden layer given the next layer's delta.
void delta_hidden_layer(Layer *layer, Layer *next_layer,
    double *next_delta, double *out_delta);

// Apply a gradient update to weights and biases.
void update_parameters(Layer *layer, double *grad_weights,
    double *grad_biases, double learning_rate);

// Full backward pass: compute all deltas and accumulated gradients.
void backpropagation(NeuronalNetwork *nn, double *input, double *expected_output,
    double **deltas, double **grad_weights, double **grad_biases,
    double **conv_grad_kernels, double **conv_grad_bias);

// Allocate zeroed gradient/delta arrays sized to the network.
void get_empty_deltas(NeuronalNetwork *nn, double ***out_deltas);
void get_empty_gradients(NeuronalNetwork *nn,
    double ***out_gradient_weights, double ***out_gradient_biases);
```

---

## Training

Defined in `include/nn/train.h`, implemented in `src/nn/train.c`.

```c
// Train nn for `epochs` passes over `dataset` with mini-batches of `batch_size`.
// Sends SIGINT (Ctrl-C) to stop early via the global stop_requested flag.
void train_nn(NeuronalNetwork *nn, Dataset *dataset, size_t epochs, size_t batch_size);

// Free all memory inside a Dataset (not the pointer itself).
void free_dataset(Dataset *dataset);
```

The global `volatile sig_atomic_t stop_requested` flag is set by `global_sigint_handler`. Register it with `signal(SIGINT, global_sigint_handler)` before calling `train_nn` if you want graceful interruption.

The default learning rate is `LEARNING_RATE` (defined in `include/nn/config.h`, currently `0.1`). Override it by setting `nn->learning_rate` before training.

---

## Accuracy Metrics

Defined in `include/nn/accuracy_metrics.h`, implemented in `src/nn/accuracy_metrics.c`.

```c
typedef struct {
    double accuracy;           // 0.0 – 1.0
    double mse;                // mean squared error
    int correct_predictions;
} EvaluationMetrics;

// Run inference on every sample in dataset and return aggregate metrics.
EvaluationMetrics evaluate_network(NeuronalNetwork *nn, Dataset *dataset);

// Append a CSV line "epoch,accuracy,mse\n" to the given file.
void log_metrics(const char *filepath, size_t epoch, EvaluationMetrics metrics);
```

---

## Model I/O

Defined in `include/nn/network_io.h`, implemented in `src/nn/network_io.c`.

Models are stored in a custom binary format identified by the four-byte magic `NNET`.

```
NNET
{n_conv_layers}
  // for each conv layer:
  input_channels input_height input_width
  n_filters kernel_height kernel_width stride padding
  kernels[]  biases[]
{n_dense_layers}
  // for each dense layer:
  n_neurons activation_type
  biases[]  weights[]
```

```c
ErrorCode save_nn(const char *path, const NeuronalNetwork *nn);
ErrorCode load_nn(const char *path, NeuronalNetwork *out_nn);
```

Both return `NN_ERR_OK` (0) on success and a negative `ErrorCode` on failure.

---

## UI Callbacks and Background Training

Defined in `include/nn/callback_for_ui.h`, implemented in `src/nn/callback_for_ui.c`.

This sub-module runs training on a POSIX thread so the GTK main loop stays responsive.

```c
typedef struct {
    char *model_path;      // NULL → create a new network
    char *dataset_folder;  // path to folder with A–Z sub-folders of bitmaps
    char *save_folder;     // where to save intermediate models
    int max_epochs;
    int save_interval;     // save every N epochs
    void (*callback)(EvaluationMetrics *metrics);  // called after each epoch
    void (*stop_cb)();                             // called when training ends
} TrainingConfig;

// Spawn background training thread. Returns 0 on success.
int start_training(TrainingConfig *config, TrainingState **state);

// Signal the training thread to stop after the current epoch.
void stop_training(TrainingState *state);

// Free all resources after the thread has finished.
void cleanup_training_state(TrainingState *state);
```

---

## Grid Builder

Defined in `include/nn/grid_builder.h`, implemented in `src/nn/grid_builder.c`.

The grid builder is the high-level OCR orchestrator. It ties together image loading, zone detection, character segmentation, CNN inference, and result assembly.

### Main pipeline function

```c
GridResult process_crossword_image(
    const char *image_path,
    const char *model_path,
    char ***out_words,
    int *out_num_words,
    int *error_code        // CrosswordError, may be NULL
);
```

Returns a `GridResult` on the heap. The caller must free it with `free_crossword_result(&result, words)`.

### `GridResult`

```c
typedef struct {
    CharBBox **grid;   // 2D array of character bounding boxes [height][width]
    char *char_grid;   // flattened character predictions (null-terminated)
    char **words;      // word-list strings (null-terminated array)
    int height;
    int width;
} GridResult;
```

### Alternative API

```c
// Fills individual output pointers; caller manages all memory.
int extract_crossword_data(
    const char *image_path, const char *model_path,
    CharBBox ***out_grid, char **out_char_grid,
    int *out_height, int *out_width,
    char ***out_words, int *out_num_words
);
```

### Lower-level helpers

```c
// Classify an array of character bounding boxes using a loaded network.
char *classify_characters(NeuronalNetwork *nn, MagickWand *wand,
    CharBBox *chars, int num);

// Assemble a GridResult from raw detections and predictions.
GridResult build_grid_for_ui(CharBBox *chars, int num, char *predictions);

// Extract the word-list strings.
char **build_wordlist_for_ui(CharBBox *chars, int num,
    char *predictions, int *num_words);
```

### Error codes

```c
typedef enum {
    CROSSWORD_OK = 0,
    CROSSWORD_ERR_IMAGE_LOAD = 1,
    CROSSWORD_ERR_MODEL_LOAD = 2,
    CROSSWORD_ERR_NO_GRID = 3,
    CROSSWORD_ERR_NO_WORDS = 4,
    CROSSWORD_ERR_MAGICK_INIT = 5,
} CrosswordError;
```

---

## Error Codes

Defined in `include/nn/errors.h`.

| Code | Value | Meaning |
|---|---|---|
| `NN_ERR_OK` | 0 | Success |
| `NN_ERR_FILE_OPEN` | −1001 | Could not open file |
| `NN_ERR_FORMAT` | −1002 | File format mismatch (bad magic or corrupt data) |
| `NN_ERR_MEMORY` | −1003 | `malloc` returned NULL |
| `NN_ERR_READ` | −1004 | `fread` returned fewer bytes than expected |
| `NN_ERR_WRITE` | −1005 | `fwrite` returned fewer bytes than expected |
| `NN_ERR_NULL_POINTER` | −1010 | A required pointer argument was NULL |

```c
void print_nn_error(ErrorCode err, const char *context);
const char *nn_error_to_string(ErrorCode err);
```

---

## Configuration

`include/nn/config.h` exposes compile-time constants:

```c
#define LEARNING_RATE 0.1
```

Override the effective learning rate at runtime by setting `nn->learning_rate` after `create_nn` / `create_cnn`.
