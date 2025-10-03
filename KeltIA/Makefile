# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -fsanitize=address -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

SRC_DIR = src
BUILD_DIR = build

SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

TARGET = ketlia

all: $(BUILD_DIR) $(TARGET)

# Creation of build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilation of objets, creation of necesary folders for building
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Edition de liens
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean
