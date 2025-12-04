CC = gcc

MAGICK_CFLAGS := $(shell pkg-config --cflags MagickWand)
MAGICK_LIBS   := $(shell pkg-config --libs   MagickWand)
GTK_CFLAGS 		:= $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS			:= $(shell pkg-config --libs   gtk+-3.0)

OPENBLAS_CFLAGS := $(shell pkg-config --cflags openblas 2>/dev/null || true)
OPENBLAS_LIBS   := $(shell pkg-config --libs openblas   2>/dev/null || echo -lopenblas)

CFLAGS = -Iinclude -Wall -Wextra -Werror -g -O2 $(MAGICK_CFLAGS) $(GTK_CFLAGS)
LDFLAGS = -lm -fsanitize=address $(MAGICK_LIBS) $(GTK_LIBS) $(OPENBLAS_LIBS)

# GTK create memory leaks unrelated to our code
# So we need separeted linker arguments for window demo
WINDOW_LDFLAGS = -lm $(MAGICK_LIBS) $(GTK_LIBS) $(OPENBLAS_LIBS)

SRC_DIR := src
BUILD_DIR := build
BINS_DIR := demos
BIN_DIR := bin

# Automatically find all executables in demos/
BIN_NAMES := $(patsubst $(BINS_DIR)/%.c,%,$(wildcard $(BINS_DIR)/*.c))
BINS := $(addprefix $(BIN_DIR)/,$(BIN_NAMES))

# Find all common code in src/ EXCEPT window.c
SRCS := $(shell find $(SRC_DIR) -name '*.c' ! -path '*/window/window.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Window-specific object
WINDOW_OBJ := $(BUILD_DIR)/window/window.o

# GResource only for window
GRESOURCE_XML := keltia.gresource.xml
GRESOURCE_C := $(BUILD_DIR)/resources.c
GRESOURCE_H := $(BUILD_DIR)/resources.h
RESOURCE_OBJ := $(BUILD_DIR)/resources.o

all: $(BINS)

# Generate resources.c
$(GRESOURCE_C): $(GRESOURCE_XML)
	@mkdir -p $(BUILD_DIR)
	glib-compile-resources --generate-source --target=$@ $<

# Generate resources.h
$(GRESOURCE_H): $(GRESOURCE_XML)
	@mkdir -p $(BUILD_DIR)
	glib-compile-resources --generate-header --target=$@ $<

# Compile resources.o
$(RESOURCE_OBJ): $(GRESOURCE_C) $(GRESOURCE_H)
	$(CC) $(CFLAGS) -c $(GRESOURCE_C) -o $@

# Compile window.o separately
$(WINDOW_OBJ): $(SRC_DIR)/window/window.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# IMPORTANT: Special rule for window
$(BIN_DIR)/window: $(BINS_DIR)/window.c $(OBJS) $(WINDOW_OBJ) $(RESOURCE_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< $(OBJS) $(WINDOW_OBJ) $(RESOURCE_OBJ) -o $@ $(WINDOW_LDFLAGS)

# Special compilation rules for NN files with CBLAS
$(BUILD_DIR)/nn/layers.o: $(SRC_DIR)/nn/layers.c | $(BUILD_DIR)/nn
	@mkdir -p $(dir $@)
	$(CC) -Iinclude -Wall -Wextra -Werror -g $(OPENBLAS_CFLAGS) -c $< -o $@

$(BUILD_DIR)/nn/back-propagation.o: $(SRC_DIR)/nn/back-propagation.c | $(BUILD_DIR)/nn
	@mkdir -p $(dir $@)
	$(CC) -Iinclude -Wall -Wextra -Werror -g $(OPENBLAS_CFLAGS) -c $< -o $@

# Generic pattern rule
$(BIN_DIR)/%: $(BINS_DIR)/%.c $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

# Pattern rule: compile any .c from src/ to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Create necessary folders
$(BUILD_DIR)/nn:
	@mkdir -p $(BUILD_DIR)/nn

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Execution targets
test: $(BIN_DIR)/tests
	$(BIN_DIR)/tests

test-solver: $(BIN_DIR)/solver
	$(BIN_DIR)/solver ./ressources/grid horizontal
	$(BIN_DIR)/solver ./ressources/grid vertical
	$(BIN_DIR)/solver ./ressources/grid diagonal
	$(BIN_DIR)/solver ./ressources/grid find
	$(BIN_DIR)/solver ./ressources/grid hello
	$(BIN_DIR)/solver ./ressources/grid world
	$(BIN_DIR)/solver ./ressources/grid goldorak
	$(BIN_DIR)/solver ./ressources/grid epita

# Cleanup
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) ressources/xor.nn

.PHONY: all test test-solver clean
