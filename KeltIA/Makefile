CC = gcc

MAGICK_CFLAGS := $(shell pkg-config --cflags MagickWand)
MAGICK_LIBS   := $(shell pkg-config --libs   MagickWand)

CFLAGS = -Iinclude -Wall -Wextra -Werror -g -O2 `pkg-config --cflags gtk+-3.0` $(MAGICK_CFLAGS)
LDFLAGS = -lm `pkg-config --libs gtk+-3.0` -fsanitize=address $(MAGICK_LIBS)

SRC_DIR := src
BUILD_DIR := build
BINS_DIR := demos
BIN_DIR := bin

# Trouver automatiquement tous les exécutables dans demos/
BIN_NAMES := $(patsubst $(BINS_DIR)/%.c,%,$(wildcard $(BINS_DIR)/*.c))
BINS := $(addprefix $(BIN_DIR)/,$(BIN_NAMES))

# Trouver tout le code commun dans src/
SRCS := $(shell find $(SRC_DIR) -name '*.c')
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(BINS)

# Règle pattern : chaque exe = main de bins/ + tous les .o de src/
$(BIN_DIR)/%: $(BINS_DIR)/%.c $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) $< $(OBJS) -o $@ $(LDFLAGS)

# Règle pattern : compiler tout .c de src/ vers .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Créer les répertoires nécessaires
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

# Cibles d'exécution
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

# Nettoyage
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR) ressources/xor.nn

.PHONY: all test test-solver clean
