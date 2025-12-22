/*
 * config_parser.h
 * Parseur de fichiers de configuration INI
 * Format moderne et extensible pour la configuration
 */

#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <windows.h>

// Structure de configuration étendue (format INI)
typedef struct {
    // [Window]
    int x;
    int y;
    int width;
    int height;
    int opacity;
    BOOL minimal_mode;
    BOOL always_on_top;

    // [Display]
    char theme[32];
    char font_name[32];
    int font_size;
    BOOL show_uptime;
    BOOL show_processes;
    BOOL show_frequency;

    // [Performance]
    int refresh_interval_ms;
    int max_disks;

    // [Hotkeys]
    int toggle_visibility_key;
    int toggle_minimal_key;
    int reload_config_key;

    // [Metrics]
    BOOL cpu_enabled;
    BOOL ram_enabled;
    BOOL disk_enabled;
    BOOL uptime_enabled;
    BOOL process_enabled;
} ConfigINI;

// Fonctions de gestion de configuration INI
void LoadConfigINI(ConfigINI* config, const char* filename);
void SaveConfigINI(const ConfigINI* config, const char* filename);
void SetDefaultConfigINI(ConfigINI* config);

// Utilitaires
void TrimWhitespace(char* str);
BOOL ParseBool(const char* value);
int ParseVirtualKey(const char* keyName);
const char* VirtualKeyToString(int vk);

#endif // CONFIG_PARSER_H
