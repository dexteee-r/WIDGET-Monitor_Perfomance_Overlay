# Makefile pour Performance Overlay
# Ce fichier automatise la compilation du programme

# Compilateur et options
CC = gcc
CFLAGS = -Wall -O2 -mwindows
LIBS = -lgdi32 -luser32 -ladvapi32 -lpsapi

# Dossiers
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Fichiers source
SOURCES = $(SRC_DIR)/main.c $(SRC_DIR)/performance.c $(SRC_DIR)/config.c $(SRC_DIR)/startup.c
OBJECTS = $(BUILD_DIR)/main.o $(BUILD_DIR)/performance.o $(BUILD_DIR)/config.o $(BUILD_DIR)/startup.o $(BUILD_DIR)/resources.o

# Nom de l'exécutable
TARGET = PerformanceOverlay_v2.exe

# Règle par défaut
all: $(BUILD_DIR) $(TARGET)

# Créer le dossier build s'il n'existe pas
$(BUILD_DIR):
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"

# Compilation de l'exécutable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
	@echo Compilation terminee! Executable: $(TARGET)

# Compilation de main.c
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de performance.c
$(BUILD_DIR)/performance.o: $(SRC_DIR)/performance.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de config.c
$(BUILD_DIR)/config.o: $(SRC_DIR)/config.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation de startup.c
$(BUILD_DIR)/startup.o: $(SRC_DIR)/startup.c
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Compilation des ressources
$(BUILD_DIR)/resources.o: resources.rc icon.ico
	windres resources.rc -o $@

# Nettoyer les fichiers compilés
clean:
	@if exist "$(BUILD_DIR)" rd /s /q "$(BUILD_DIR)"
	@if exist "$(TARGET)" del /q "$(TARGET)"
	@echo Nettoyage termine!

# Recompiler complètement
rebuild: clean all

# Lancer le programme après compilation
run: all
	@echo Lancement du programme...
	@$(TARGET)

# Créer l'icône (nécessite Python et Pillow)
icon:
	@echo Creation de l'icone...
	@python create_icon.py

.PHONY: all clean rebuild run icon
