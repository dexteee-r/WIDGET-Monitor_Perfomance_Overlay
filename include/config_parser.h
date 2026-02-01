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

    // [Animations]
    BOOL animations_enabled;
    int alert_cpu_threshold;   // Seuil alerte CPU (défaut: 80)
    int alert_ram_threshold;   // Seuil alerte RAM (défaut: 90)

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

    // [Theme]
    int theme_index;  // Index du thème sélectionné (0-5, 5=Custom)

    // [CustomTheme] - Couleurs personnalisées (format R,G,B)
    BOOL custom_theme_enabled;
    COLORREF custom_bg;
    COLORREF custom_panel;
    COLORREF custom_border;
    COLORREF custom_accent;
    COLORREF custom_accent2;
    COLORREF custom_text;
    COLORREF custom_text_muted;

    // [Prayer]
    BOOL prayer_enabled;
    BOOL prayer_use_api;      // TRUE = utiliser API, FALSE = horaires manuels
    char prayer_city[64];     // Ville pour l'API (ex: "Paris")
    char prayer_country[64];  // Pays pour l'API (ex: "France")
    int prayer_method;        // Méthode de calcul (2=ISNA, 3=MWL, 5=Egypt, etc.)
    // Horaires manuels (fallback si API désactivée ou échec)
    char prayer_fajr[8];      // Format "HH:MM"
    char prayer_dhuhr[8];
    char prayer_asr[8];
    char prayer_maghrib[8];
    char prayer_isha[8];
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
