/*
 * config.c
 * Implémentation de la gestion de configuration
 */

#include <windows.h>
#include <stdio.h>
#include "../include/config.h"

#define CONFIG_FILE "config.txt"

/*
 * LoadConfig
 * Charge la configuration depuis un fichier
 * Si le fichier n'existe pas, utilise des valeurs par défaut
 */
void LoadConfig(Config* config) {
    FILE* file = fopen(CONFIG_FILE, "r");

    if (file != NULL) {
        // Lire les valeurs depuis le fichier
        fscanf(file, "x=%d\n", &config->x);
        fscanf(file, "y=%d\n", &config->y);

        int minimal;
        fscanf(file, "minimal_mode=%d\n", &minimal);
        config->minimal_mode = (minimal != 0);

        fclose(file);
    } else {
        // Valeurs par défaut si le fichier n'existe pas
        config->x = 10;
        config->y = 10;
        config->minimal_mode = FALSE;
    }
}

/*
 * SaveConfig
 * Sauvegarde la configuration dans un fichier
 */
void SaveConfig(const Config* config) {
    FILE* file = fopen(CONFIG_FILE, "w");

    if (file != NULL) {
        // Écrire les valeurs dans le fichier
        fprintf(file, "x=%d\n", config->x);
        fprintf(file, "y=%d\n", config->y);
        fprintf(file, "minimal_mode=%d\n", config->minimal_mode ? 1 : 0);

        fclose(file);
    }
}
