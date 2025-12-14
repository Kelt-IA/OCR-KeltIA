CC = gcc

MAGICK_CFLAGS := $(shell pkg-config --cflags MagickWand)
MAGICK_LIBS := $(shell pkg-config --libs MagickWand)
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS := $(shell pkg-config --libs gtk+-3.0)

CFLAGS = -Iinclude -Wall -Wextra -Werror -g -O2 $(MAGICK_CFLAGS) $(GTK_CFLAGS)

# Use same linker flags as window (no fsanitize for GTK)
LDFLAGS = -lm $(MAGICK_LIBS) $(GTK_LIBS) $(OPENBLAS_LIBS)

SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# Find all source files in src/ EXCEPT keltia.c
SRCS := $(shell find $(SRC_DIR) -name '*.c' ! -path '*/keltia.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# GResource files
GRESOURCE_XML := keltia.gresource.xml
GRESOURCE_C := $(BUILD_DIR)/resources.c
GRESOURCE_H := $(BUILD_DIR)/resources.h
RESOURCE_OBJ := $(BUILD_DIR)/resources.o

# Target executable
TARGET := $(BIN_DIR)/keltia

all: $(TARGET)

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

# Build keltia executable from src/keltia.c
$(TARGET): src/keltia.c $(OBJS) $(RESOURCE_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< $(OBJS) $(RESOURCE_OBJ) -o $@ $(LDFLAGS)

# Special compilation rules for NN files with CBLAS
$(BUILD_DIR)/nn/layers.o: $(SRC_DIR)/nn/layers.c | $(BUILD_DIR)/nn
	@mkdir -p $(dir $@)
	$(CC) -Iinclude -Wall -Wextra -Werror -g -O2 -c $< -o $@

$(BUILD_DIR)/nn/back-propagation.o: $(SRC_DIR)/nn/back-propagation.c | $(BUILD_DIR)/nn
	@mkdir -p $(dir $@)
	$(CC) -Iinclude -Wall -Wextra -Werror -g -O2 -c $< -o $@

# Pattern rule: compile any .c from src/ to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Create necessary folders
$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/nn:
	@mkdir -p $(BUILD_DIR)/nn

# Cleanup
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all clean
