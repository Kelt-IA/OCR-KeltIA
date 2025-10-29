# OCR-KeltIA

OCR-KeltIA is an Optical Character Recognition system implemented in C, using modular neural network components. It is designed to detect text zones from images, segment characters, and recognize them through a custom neural network.

## Table of Contents

- [Introduction](#introduction)  
- [Repository Structure](#repository-structure)  
- [Features](#features)  
- [Getting Started](#getting-started)  
  - [Prerequisites](#prerequisites)  
  - [Building the Project](#building-the-project)  
- [Git Hooks](#git-hooks)  
- [License](#license)  

## Introduction

OCR-KeltIA provides end-to-end recognition from image preprocessing to neural network inference and training. The project is written in C for performance and training flexibility, consisting of modules for zone detection, neural network operations, and problem-solving.

## Repository Structure

```
.
├── AUTHORS                   # Author information
├── KeltIA
│   ├── bins                  # Main executables
│   ├── include               # Header files separated by modules
│   ├── ressources            # Sample input data (images, grids)
│   ├── src                   # Source files organized into modules
│   ├── Makefile
├── LICENSE                   # License information
├── README.md                 # This file
├── scripts                   # Git hooks and automation scripts

```


## Features

- Modular implementation of neural networks in C, including backpropagation, activation functions, and training utilities.  
- Detection and segmentation of text zones in images.  
- Ability to save and load trained neural network models.  
- Automated code formatting via Git hooks using clang-format.  
- Example training programs such as XOR neural network for testing correctness.

## Getting Started

### Prerequisites

- A C compiler supporting C99 standard (e.g., gcc, clang)  
- Make build system
- (optional) clang-format installed (for automatic code formatting)  

### Building the Project

1. cd into KeltIA/
2. execute the following command:

```bash
make all
```

This will compile all modules and produce the main binaries in `KeltIA/bins/`.


## Git hooks

To enable the hooks provided by this repo you will need to configure 
your git config, the following command will enable it for this repo.

```bash
# in the root of the repo
git config core.hooksPath scripts/git-hooks
```

### pre-commit hook

This hook is triggered before commiting any change to git, it
looks into the commited files if there is any .c or .h file and
formats it with clang-format with the following the rules found 
in the file `.clang-format` in `KeltIA/.clang-format`


## License

This project is licensed under the terms described in the LICENSE file.

