# Compilateur et options
CC = gcc
CFLAGS = -Wall -Wextra -O2 `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

# Répertoires
SRC_DIR = src
BUILD_DIR = build

# Recherche récursive des sources
SRCS = $(shell find $(SRC_DIR) -name '*.c')
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Nom de l’exécutable
TARGET = ketlia

# Règle par défaut
all: $(BUILD_DIR) $(TARGET)

# Création du dossier build s'il n'existe pas
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilation en objets, création des dossiers nécessaires dans build
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Edition de liens
$(TARGET): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

# Nettoyage
clean:
	rm -rf $(BUILD_DIR)/*.o $(TARGET)

.PHONY: all clean
