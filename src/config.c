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

        // Validation : Vérifier que les coordonnées sont dans les limites de l'écran
        int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
        int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);

        // Si la fenêtre est complètement hors écran, la réinitialiser
        if (config->x < screenLeft - 100 || config->x > screenWidth + screenLeft ||
            config->y < screenTop - 100 || config->y > screenHeight + screenTop) {
            // Centrer la fenêtre sur l'écran principal
            config->x = 100;
            config->y = 100;
        }

        // S'assurer que les coordonnées ne sont pas négatives sur l'écran principal
        if (config->x < 0 && config->x > -280) config->x = 0;
        if (config->y < 0 && config->y > -240) config->y = 0;

    } else {
        // Valeurs par défaut si le fichier n'existe pas
        config->x = 100;
        config->y = 100;
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
