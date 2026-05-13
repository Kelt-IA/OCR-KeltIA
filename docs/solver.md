# Solver Module

Source: `KeltIA/src/solver/`  
Headers: `KeltIA/include/solver/`

The solver module finds words inside a 2D character grid (word-search puzzle style) and reads grids from plain-text files.

---

## Table of Contents

- [solver — Word Search](#solver--word-search)
- [grid_reader — Grid File Loader](#grid_reader--grid-file-loader)

---

## solver — Word Search

Header: `include/solver/solver.h`  
Source: `src/solver/solver.c`

```c
// Bounding box of a found word expressed as two character positions.
typedef struct {
    int x1, y1;   // position of the first letter
    int x2, y2;   // position of the last letter
} WordPos;

// Search for word (length word_len) in grid.
// grid is a row-major char array of size height × width.
// Returns a heap-allocated WordPos on success, or NULL if the word is not found.
// The caller must free the returned pointer.
WordPos *solver(char *grid, int height, int width, const char *word, int word_len);
```

### Search directions

The solver checks all 8 directions from every cell:

| Direction | dx | dy |
|---|---|---|
| Right | +1 | 0 |
| Left | −1 | 0 |
| Down | 0 | +1 |
| Up | 0 | −1 |
| Down-right | +1 | +1 |
| Down-left | −1 | +1 |
| Up-right | +1 | −1 |
| Up-left | −1 | −1 |

### Usage example

```c
char *grid = "HELLOWORLDABCDE";
int h = 3, w = 5;
WordPos *pos = solver(grid, h, w, "HELLO", 5);
if (pos) {
    // highlight from (pos->x1, pos->y1) to (pos->x2, pos->y2)
    free(pos);
}
```

---

## grid_reader — Grid File Loader

Header: `include/solver/grid_reader.h`  
Source: `src/solver/grid_reader.c`

Reads a plain-text crossword grid file into a 2D array of characters.

```c
// Read a grid file from filename.
// Each line of the file is one row; rows must all have the same length.
// Sets *rows and *cols to the grid dimensions.
// Returns a heap-allocated array of heap-allocated row strings.
// Caller must free with free_grid().
char **readFile(const char *filename, int *rows, int *cols);

// Free the array returned by readFile.
void free_grid(char **grid);
```

### Grid file format

A plain-text file where each line is a row of the crossword grid:

```
CROSSWORD
EXAMPLE_A
TESTBOARD
```

Rows must all be the same length. Letters are case-sensitive in the solver; convert to uppercase before searching if needed.
