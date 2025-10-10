CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS = -lm `pkg-config --libs gtk+-3.0` -fsanitize=address

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
TEST_BUILD_DIR = build/tests
SOLVER_BUILD_DIR = build/solver

# executable files
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Test source files
TEST_SRCS = $(shell find $(TEST_DIR) -name '*.c')
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))

# Source objects excluding main.c to avoid conflict with test main
SRC_OBJS_NO_MAIN = $(filter-out $(BUILD_DIR)/main.o,$(OBJS))

# Source objects excluding both main.c and solver.c for main executable
SRC_OBJS_MAIN = $(filter-out $(BUILD_DIR)/solver/solver.o,$(OBJS))

# Source objects excluding main.c but including solver.c for solver executable
SRC_OBJS_SOLVER = $(filter-out $(BUILD_DIR)/main.o,$(OBJS))

TARGET = ketlia
TEST_TARGET = test_executable
SOLVER_TARGET = solver

all: $(BUILD_DIR) $(TARGET)

# Creation of build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilation of objets, creation of necesary folders for building
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Final executable (excludes solver.c)
$(TARGET): $(SRC_OBJS_MAIN)
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


# --- SOLVER ---

# Create solver build directory
$(SOLVER_BUILD_DIR):
	mkdir -p $(SOLVER_BUILD_DIR)

# Solver executable (uses all objects except main.o)
$(SOLVER_TARGET): $(BUILD_DIR) $(SRC_OBJS_SOLVER)
	$(CC) $(SRC_OBJS_SOLVER) -o $@ $(LDFLAGS)

# Target to compile and run solver
test-solver: $(SOLVER_TARGET)
	./tests/solver/test_solver.sh


# --- CLEAN TARGETS ---

# Clean main program build files
clean-main:
	rm -rf $(BUILD_DIR)/nn/*.o $(BUILD_DIR)/main.o $(TARGET)

# Clean test build files
clean-test:
	rm -rf $(TEST_BUILD_DIR) $(TEST_TARGET)
	rm -f xor.nn

# Clean solver build files
clean-solver:
	rm -rf $(SOLVER_TARGET)

# Clean everything
clean: clean-main clean-test clean-solver
	rm -rf $(BUILD_DIR)

.PHONY: all clean clean-main clean-test clean-solver test test-solver
