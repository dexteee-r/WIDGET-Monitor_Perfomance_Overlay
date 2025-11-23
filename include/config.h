/*
 * config.h
 * En-tête pour la gestion de la configuration
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <windows.h>

// Structure de configuration
typedef struct {
    int x;                  // Position X de la fenêtre
    int y;                  // Position Y de la fenêtre
    BOOL minimal_mode;      // Mode minimaliste (TRUE ou FALSE)
} Config;

// Fonctions de gestion de configuration
void LoadConfig(Config* config);
void SaveConfig(const Config* config);

#endif // CONFIG_H