CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS = -lm `pkg-config --libs gtk+-3.0` -fsanitize=address

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
TEST_BUILD_DIR = build/tests

# executable files
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Test source files
TEST_SRCS = $(shell find $(TEST_DIR) -name '*.c')
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))

# Source objects excluding main.c to avoid conflict with test main
SRC_OBJS_NO_MAIN = $(filter-out $(BUILD_DIR)/main.o,$(OBJS))

TARGET = ketlia
TEST_TARGET = test_executable

all: $(BUILD_DIR) $(TARGET)

# Creation of build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilation of objets, creation of necesary folders for building
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Final executable
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)


# --- TESTS ---

# Create test build directory
$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

# Compile test files
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link test executable (links with src objects except main.o)
$(TEST_TARGET): $(TEST_BUILD_DIR) $(SRC_OBJS_NO_MAIN) $(TEST_OBJS)
	$(CC) $(TEST_OBJS) $(SRC_OBJS_NO_MAIN) -o $@ $(LDFLAGS)

# Target to compile and run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)

# Clean main program build files
clean-main:
	rm -rf $(BUILD_DIR)/nn/*.o $(BUILD_DIR)/main.o $(TARGET)

# Clean test build files
clean-test:
	rm -rf $(TEST_BUILD_DIR) $(TEST_TARGET)
	rm xor.nn

# Clean everything
clean: clean-main clean-test

.PHONY: all clean
