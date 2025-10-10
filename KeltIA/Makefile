CC = gcc
CFLAGS = -Iinclude -Wall -Wextra -g -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS = -lm `pkg-config --libs gtk+-3.0` -fsanitize=address

SRC_DIR = src
BUILD_DIR = build
TEST_DIR = tests
TEST_BUILD_DIR = build/tests

# executable files - todos los sources y objetos
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Test source files
TEST_SRCS = $(shell find $(TEST_DIR) -name '*.c')
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(TEST_BUILD_DIR)/%.o,$(TEST_SRCS))

# ========== INDIVIDUAL MAINS ==========
MAIN_OBJ = $(BUILD_DIR)/main.o
SOLVER_MAIN_OBJ = $(BUILD_DIR)/solver/main.o
TEST_MAIN_OBJ = $(TEST_BUILD_DIR)/main.o

# ========== COMMON BASE (no main files) ==========
SRC_OBJS_NO_MAINS = $(filter-out $(MAIN_OBJ) $(SOLVER_MAIN_OBJ), $(OBJS))

# Main executable: common base + main.o
OBJS_FOR_MAIN = $(SRC_OBJS_NO_MAINS) $(MAIN_OBJ)

# Solver executable: common base + main.o (from src/solver/)
OBJS_FOR_SOLVER = $(SRC_OBJS_NO_MAINS) $(SOLVER_MAIN_OBJ)

# Test executable: common base + main.o (from tests/)
OBJS_FOR_TESTS = $(SRC_OBJS_NO_MAINS) $(TEST_OBJS)

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

# Final executable (base + main.o)
$(TARGET): $(OBJS_FOR_MAIN)
	$(CC) $^ -o $@ $(LDFLAGS)


# --- TESTS ---

# Create test build directory
$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

# Compile test files
$(TEST_BUILD_DIR)/%.o: $(TEST_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Link test executable (base + test objects, NO incluye main.o ni main_solver.o)
$(TEST_TARGET): $(TEST_BUILD_DIR) $(OBJS_FOR_TESTS)
	$(CC) $(OBJS_FOR_TESTS) -o $@ $(LDFLAGS)

# Target to compile and run tests
test: $(TEST_TARGET)
	./$(TEST_TARGET)


# --- SOLVER ---

# Solver executable (base + main_solver.o, NO incluye main.o)
$(SOLVER_TARGET): $(BUILD_DIR) $(OBJS_FOR_SOLVER)
	$(CC) $(OBJS_FOR_SOLVER) -o $@ $(LDFLAGS)

# Target to compile and run solver
test-solver: $(SOLVER_TARGET)
	./tests/solver/test_solver.sh


# --- CLEAN TARGETS ---

# Clean main program build files
clean-main:
	rm -rf $(BUILD_DIR)/nn/*.o $(MAIN_OBJ) $(TARGET)

# Clean test build files
clean-test:
	rm -rf $(TEST_BUILD_DIR) $(TEST_TARGET)
	rm -f xor.nn

# Clean solver build files
clean-solver:
	rm -rf $(SOLVER_MAIN_OBJ) $(SOLVER_TARGET)

# Clean everything
clean: clean-main clean-test clean-solver
	rm -rf $(BUILD_DIR)

.PHONY: all clean clean-main clean-test clean-solver test test-solver
