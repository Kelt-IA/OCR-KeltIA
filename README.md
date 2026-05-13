# OCR-KeltIA

OCR-KeltIA is an Optical Character Recognition system implemented in C that recognizes crossword puzzle images. It preprocesses images, detects text zones, segments individual characters, classifies them with a custom convolutional neural network, and then solves the puzzle — all through a GTK3 graphical interface.

## Table of Contents

- [Introduction](#introduction)
- [Repository Structure](#repository-structure)
- [Features](#features)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Building the Project](#building-the-project)
  - [Running](#running)
- [Demos](#demos)
- [Git Hooks](#git-hooks)
- [Documentation](#documentation)
- [License](#license)

## Introduction

OCR-KeltIA provides an end-to-end pipeline from raw image to solved crossword:

1. **Preprocessing** — grayscale conversion, binarization, noise removal, and automatic deskewing via ImageMagick.
2. **Zone detection** — projection-based algorithm that separates the grid zone from the word-list zone.
3. **Character segmentation** — flood-fill connected-component analysis that finds each individual letter's bounding box.
4. **Classification** — a custom CNN written in pure C infers the character class (A–Z).
5. **Solving** — a brute-force word-search solver finds each word in the character grid and highlights its position.
6. **GUI** — a GTK3 interface ties everything together and allows loading images, loading or training a model, and displaying results.

## Repository Structure

```
.
├── AUTHORS
├── KeltIA/
│   ├── demos/              # Standalone programs (each compiles to its own binary)
│   ├── include/            # Public headers, mirroring src/ layout
│   │   ├── detect_zones/
│   │   ├── image/
│   │   ├── io/
│   │   ├── nn/
│   │   ├── solver/
│   │   ├── tests/
│   │   └── window/
│   ├── ressources/         # Sample images and grids for manual testing
│   ├── src/                # Library source files, organized by module
│   │   ├── detect_zones/
│   │   ├── image/
│   │   ├── io/
│   │   ├── nn/
│   │   ├── solver/
│   │   ├── tests/
│   │   ├── window/
│   │   └── keltia.c        # Application entry point
│   └── Makefile
├── docs/                   # Detailed per-module documentation
├── scripts/
│   └── git-hooks/
├── LICENSE
└── README.md
```

## Features

- Custom neural network engine in C with dense and convolutional layers, backpropagation, and mini-batch SGD.
- Activation functions: sigmoid, leaky ReLU, softmax, step.
- Binary `.nn` model format for saving and loading trained networks.
- Image preprocessing pipeline (grayscale, binarization, noise removal, auto-rotation) built on ImageMagick.
- Projection-based zone detection to split crossword images into grid and word-list regions.
- Flood-fill character segmentation that produces per-character bounding boxes.
- MNIST/EMNIST and bitmap folder dataset loaders for training.
- Background training thread with real-time accuracy/MSE callbacks to the UI.
- GTK3 GUI for loading images, loading/training models, and viewing results.
- Sudoku-style word-search solver.
- Pre-commit git hook for automatic `clang-format` formatting.

## Getting Started

### Prerequisites

| Dependency | Purpose |
|---|---|
| `gcc` or `clang` (C99) | Compiler |
| `make` | Build system |
| `ImageMagick` (MagickWand) | Image processing |
| `GTK+ 3` | Graphical interface |
| `glib-compile-resources` | Bundling UI resources (part of GLib) |
| `clang-format` *(optional)* | Code formatting via git hook |

On Debian/Ubuntu:

```bash
sudo apt install gcc make libmagickwand-dev libgtk-3-dev libglib2.0-dev-bin
```

On Arch Linux:

```bash
sudo pacman -S gcc make imagemagick gtk3
```

### Building the Project

```bash
cd KeltIA
make all
```

This produces `KeltIA/bin/keltia`.

To remove build artifacts:

```bash
make clean
```

### Running

```bash
./bin/keltia
```

The GUI will open. From there you can:

1. **Open image** — load a crossword puzzle image.
2. **Open model** — load a pre-trained `.nn` model file.
3. **Solve** — run the full OCR and solver pipeline on the loaded image.
4. **Open training** — launch the training panel to train a new model on a bitmap dataset.

## Demos

The `demos/` directory contains small standalone programs that exercise individual parts of the pipeline:

| Demo | Description |
|---|---|
| `apply_autorotation` | Auto-detect and correct image skew |
| `apply_rotation` | Rotate an image by a given angle |
| `apply_traitement` | Apply the full preprocessing pipeline |
| `cnn_training_bitmaps` | Train the CNN on a bitmap folder dataset |
| `construct_grid_and_word_list` | Run the full OCR pipeline from the command line |
| `detect_char` | Run character segmentation on a zone image |
| `detect_zones` | Run zone detection and output bounding boxes |
| `detect_zones_and_letters` | Detect zones and then segment characters |
| `image_correction` | Apply selective preprocessing steps |
| `mnist_leters_training` | Train on an EMNIST letters dataset |
| `preprocessing` | Preview preprocessing steps individually |
| `solver` | Run the word-search solver on a text grid |
| `tests` | Run the unit test suite |
| `window` | Launch the GTK window (same as the main binary) |

## Git Hooks

To enable the provided hooks, run once from the repo root:

```bash
git config core.hooksPath scripts/git-hooks
```

### pre-commit

Before every commit the hook scans staged files for `.c` and `.h` files and formats them with `clang-format` using the rules in `KeltIA/.clang-format`.

## Documentation

Detailed per-module reference is in the [`docs/`](docs/) directory:

- [`docs/nn.md`](docs/nn.md) — Neural network engine (layers, training, I/O)
- [`docs/image.md`](docs/image.md) — Image preprocessing pipeline
- [`docs/detect_zones.md`](docs/detect_zones.md) — Zone and character detection
- [`docs/io.md`](docs/io.md) — Dataset loaders and I/O utilities
- [`docs/solver.md`](docs/solver.md) — Word-search solver
- [`docs/window.md`](docs/window.md) — GTK3 GUI

## License

This project is licensed under the terms described in the [LICENSE](LICENSE) file.
