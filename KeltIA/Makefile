CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -Werror -g -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS = -lm `pkg-config --libs gtk+-3.0` -fsanitize=address

SRC_DIR := src
BUILD_DIR := build
BINS_DIR := bins

# Automatically find all executables in bins/
BINS := $(patsubst $(BINS_DIR)/%.c,%,$(wildcard $(BINS_DIR)/*.c))

# Find all common code in src/
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(BINS)

# Pattern rule: each exe = main from bins/ + every .o from src/
%: $(BINS_DIR)/%.c $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

# Pattern rule: compile any .c from src/ to .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Execution targets
test: tests
	./tests

test-solver: solver
	./solver ./ressources/grid horizontal
	./solver ./ressources/grid vertical
	./solver ./ressources/grid diagonal
	./solver ./ressources/grid find
	./solver ./ressources/grid hello
	./solver ./ressources/grid world
	./solver ./ressources/grid goldorak
	./solver ./ressources/grid epita

# Clean
clean:
	rm -rf $(BUILD_DIR) $(BINS) ressources/xor.nn

.PHONY: all test test-solver clean
