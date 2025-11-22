CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Isrc
SRC_DIR = src
POLITIQUES_DIR = politiques
BUILD_DIR = build

# Fichiers source
SRC = $(SRC_DIR)/main.c $(SRC_DIR)/menu.c $(SRC_DIR)/process.c
POLITIQUES_SRC = $(POLITIQUES_DIR)/multi_level.c $(POLITIQUES_DIR)/fifo.c $(POLITIQUES_DIR)/round_robin.c $(POLITIQUES_DIR)/priorite.c

# Fichiers objets dans build/
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
POLITIQUES_OBJ = $(POLITIQUES_SRC:$(POLITIQUES_DIR)/%.c=$(BUILD_DIR)/%.o)

EXEC = ordonnanceur

all: $(BUILD_DIR) $(EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXEC): $(OBJ) $(POLITIQUES_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(POLITIQUES_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(EXEC)

install: $(EXEC)
	@if [ -w /usr/local/bin ]; then \
		cp $(EXEC) /usr/local/bin/; \
		echo "Installé dans /usr/local/bin"; \
	else \
		echo "Installé dans le répertoire courant (pas de droits admin)"; \
	fi

.PHONY: all clean install
