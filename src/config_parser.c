/*
 * config_parser.c
 * Implémentation du parseur de configuration INI
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "../include/config_parser.h"
#include "../include/constants.h"

/*
 * TrimWhitespace
 * Enlève les espaces en début et fin de chaîne
 */
void TrimWhitespace(char* str) {
    if (!str) return;

    // Trim début
    char* start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // Trim fin
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    // Copier le résultat
    size_t len = (end - start + 1);
    memmove(str, start, len);
    str[len] = '\0';
}

/*
 * ParseBool
 * Convertit une chaîne en booléen
 */
BOOL ParseBool(const char* value) {
    if (!value) return FALSE;

    char lower[32];
    strncpy(lower, value, sizeof(lower) - 1);
    lower[sizeof(lower) - 1] = '\0';

    // Convertir en minuscules
    for (int i = 0; lower[i]; i++) {
        lower[i] = (char)tolower((unsigned char)lower[i]);
    }

    return (strcmp(lower, "true") == 0 || strcmp(lower, "1") == 0 || strcmp(lower, "yes") == 0);
}

/*
 * ParseVirtualKey
 * Convertit un nom de touche en code VK
 */
int ParseVirtualKey(const char* keyName) {
    if (!keyName) return 0;

    if (strcmp(keyName, "F1") == 0) return VK_F1;
    if (strcmp(keyName, "F2") == 0) return VK_F2;
    if (strcmp(keyName, "F3") == 0) return VK_F3;
    if (strcmp(keyName, "F4") == 0) return VK_F4;
    if (strcmp(keyName, "F5") == 0) return VK_F5;
    if (strcmp(keyName, "F6") == 0) return VK_F6;
    if (strcmp(keyName, "F7") == 0) return VK_F7;
    if (strcmp(keyName, "F8") == 0) return VK_F8;
    if (strcmp(keyName, "F9") == 0) return VK_F9;
    if (strcmp(keyName, "F10") == 0) return VK_F10;
    if (strcmp(keyName, "F11") == 0) return VK_F11;
    if (strcmp(keyName, "F12") == 0) return VK_F12;

    return 0;
}

/*
 * ParseRGB
 * Convertit une chaîne "R,G,B" en COLORREF
 */
static COLORREF ParseRGB(const char* value) {
    int r = 0, g = 0, b = 0;
    sscanf(value, "%d,%d,%d", &r, &g, &b);
    // Clamp values
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    return RGB(r, g, b);
}

/*
 * VirtualKeyToString
 * Convertit un code VK en nom de touche
 */
const char* VirtualKeyToString(int vk) {
    switch (vk) {
        case VK_F1: return "F1";
        case VK_F2: return "F2";
        case VK_F3: return "F3";
        case VK_F4: return "F4";
        case VK_F5: return "F5";
        case VK_F6: return "F6";
        case VK_F7: return "F7";
        case VK_F8: return "F8";
        case VK_F9: return "F9";
        case VK_F10: return "F10";
        case VK_F11: return "F11";
        case VK_F12: return "F12";
        default: return "Unknown";
    }
}

/*
 * SetDefaultConfigINI
 * Définit les valeurs par défaut de la configuration
 */
void SetDefaultConfigINI(ConfigINI* config) {
    // [Window]
    config->x = 10;
    config->y = 10;
    config->width = WINDOW_WIDTH;
    config->height = WINDOW_HEIGHT_FULL;
    config->opacity = WINDOW_OPACITY;
    config->minimal_mode = FALSE;
    config->always_on_top = TRUE;

    // [Display]
    strcpy(config->theme, "neon_dark");
    strcpy(config->font_name, FONT_NORMAL_NAME);
    config->font_size = FONT_NORMAL_SIZE;
    config->show_uptime = TRUE;
    config->show_processes = TRUE;
    config->show_frequency = TRUE;

    // [Animations]
    config->animations_enabled = TRUE;
    config->alert_cpu_threshold = 80;
    config->alert_ram_threshold = 90;

    // [Performance]
    config->refresh_interval_ms = TIMER_INTERVAL;
    config->max_disks = MAX_DISKS;

    // [Hotkeys]
    config->toggle_visibility_key = VK_F3;
    config->toggle_minimal_key = VK_F2;
    config->reload_config_key = VK_F5;

    // [Metrics]
    config->cpu_enabled = TRUE;
    config->ram_enabled = TRUE;
    config->disk_enabled = TRUE;
    config->uptime_enabled = TRUE;
    config->process_enabled = TRUE;

    // [Theme]
    config->theme_index = 0;  // Neon Dark par défaut

    // [CustomTheme] - Valeurs par défaut (style violet/rose)
    config->custom_theme_enabled = FALSE;
    config->custom_bg = RGB(20, 10, 30);
    config->custom_panel = RGB(30, 20, 45);
    config->custom_border = RGB(60, 40, 80);
    config->custom_accent = RGB(200, 100, 255);
    config->custom_accent2 = RGB(255, 100, 150);
    config->custom_text = RGB(230, 220, 255);
    config->custom_text_muted = RGB(140, 120, 160);

    // [Prayer]
    config->prayer_enabled = TRUE;
    config->prayer_use_api = TRUE;
    strcpy(config->prayer_city, "Brussels");
    strcpy(config->prayer_country, "Belgium");
    config->prayer_method = 2;  // ISNA (Islamic Society of North America)
    // Horaires manuels par défaut (fallback)
    strcpy(config->prayer_fajr, "06:00");
    strcpy(config->prayer_dhuhr, "13:00");
    strcpy(config->prayer_asr, "16:00");
    strcpy(config->prayer_maghrib, "19:00");
    strcpy(config->prayer_isha, "21:00");
}

/*
 * LoadConfigINI
 * Charge la configuration depuis un fichier INI
 */
void LoadConfigINI(ConfigINI* config, const char* filename) {
    // Charger les valeurs par défaut d'abord
    SetDefaultConfigINI(config);

    FILE* file = fopen(filename, "r");
    if (!file) {
        return;  // Fichier n'existe pas, utiliser les défauts
    }

    char line[256];
    char section[64] = "";

    while (fgets(line, sizeof(line), file)) {
        // Enlever le \n
        line[strcspn(line, "\r\n")] = 0;

        // Trim
        TrimWhitespace(line);

        // Ignorer commentaires et lignes vides
        if (line[0] == ';' || line[0] == '#' || line[0] == '\0') {
            continue;
        }

        // Détecter section [XXX]
        if (line[0] == '[') {
            sscanf(line, "[%63[^]]]", section);
            TrimWhitespace(section);
            continue;
        }

        // Parser clé=valeur
        char key[64], value[128];
        char* equals = strchr(line, '=');
        if (!equals) continue;

        // Extraire clé et valeur
        size_t key_len = equals - line;
        if (key_len >= sizeof(key)) key_len = sizeof(key) - 1;
        strncpy(key, line, key_len);
        key[key_len] = '\0';
        TrimWhitespace(key);

        strncpy(value, equals + 1, sizeof(value) - 1);
        value[sizeof(value) - 1] = '\0';
        TrimWhitespace(value);

        // Parser selon la section
        if (strcmp(section, "Window") == 0) {
            if (strcmp(key, "x") == 0) config->x = atoi(value);
            else if (strcmp(key, "y") == 0) config->y = atoi(value);
            else if (strcmp(key, "width") == 0) config->width = atoi(value);
            else if (strcmp(key, "height") == 0) config->height = atoi(value);
            else if (strcmp(key, "opacity") == 0) config->opacity = atoi(value);
            else if (strcmp(key, "minimal_mode") == 0) config->minimal_mode = ParseBool(value);
            else if (strcmp(key, "always_on_top") == 0) config->always_on_top = ParseBool(value);
        }
        else if (strcmp(section, "Display") == 0) {
            if (strcmp(key, "theme") == 0) {
                strncpy(config->theme, value, sizeof(config->theme) - 1);
                config->theme[sizeof(config->theme) - 1] = '\0';
            }
            else if (strcmp(key, "font_name") == 0) {
                strncpy(config->font_name, value, sizeof(config->font_name) - 1);
                config->font_name[sizeof(config->font_name) - 1] = '\0';
            }
            else if (strcmp(key, "font_size") == 0) config->font_size = atoi(value);
            else if (strcmp(key, "show_uptime") == 0) config->show_uptime = ParseBool(value);
            else if (strcmp(key, "show_processes") == 0) config->show_processes = ParseBool(value);
            else if (strcmp(key, "show_frequency") == 0) config->show_frequency = ParseBool(value);
        }
        else if (strcmp(section, "Animations") == 0) {
            if (strcmp(key, "enabled") == 0) config->animations_enabled = ParseBool(value);
            else if (strcmp(key, "alert_cpu_threshold") == 0) config->alert_cpu_threshold = atoi(value);
            else if (strcmp(key, "alert_ram_threshold") == 0) config->alert_ram_threshold = atoi(value);
        }
        else if (strcmp(section, "Performance") == 0) {
            if (strcmp(key, "refresh_interval_ms") == 0) config->refresh_interval_ms = atoi(value);
            else if (strcmp(key, "max_disks") == 0) config->max_disks = atoi(value);
        }
        else if (strcmp(section, "Hotkeys") == 0) {
            if (strcmp(key, "toggle_visibility") == 0) config->toggle_visibility_key = ParseVirtualKey(value);
            else if (strcmp(key, "toggle_minimal") == 0) config->toggle_minimal_key = ParseVirtualKey(value);
            else if (strcmp(key, "reload_config") == 0) config->reload_config_key = ParseVirtualKey(value);
        }
        else if (strcmp(section, "Metrics") == 0) {
            if (strcmp(key, "cpu_enabled") == 0) config->cpu_enabled = ParseBool(value);
            else if (strcmp(key, "ram_enabled") == 0) config->ram_enabled = ParseBool(value);
            else if (strcmp(key, "disk_enabled") == 0) config->disk_enabled = ParseBool(value);
            else if (strcmp(key, "uptime_enabled") == 0) config->uptime_enabled = ParseBool(value);
            else if (strcmp(key, "process_enabled") == 0) config->process_enabled = ParseBool(value);
        }
        else if (strcmp(section, "Theme") == 0) {
            if (strcmp(key, "index") == 0) {
                config->theme_index = atoi(value);
                if (config->theme_index < 0 || config->theme_index > 5) {
                    config->theme_index = 0;
                }
            }
        }
        else if (strcmp(section, "CustomTheme") == 0) {
            if (strcmp(key, "enabled") == 0) config->custom_theme_enabled = ParseBool(value);
            else if (strcmp(key, "bg") == 0) config->custom_bg = ParseRGB(value);
            else if (strcmp(key, "panel") == 0) config->custom_panel = ParseRGB(value);
            else if (strcmp(key, "border") == 0) config->custom_border = ParseRGB(value);
            else if (strcmp(key, "accent") == 0) config->custom_accent = ParseRGB(value);
            else if (strcmp(key, "accent2") == 0) config->custom_accent2 = ParseRGB(value);
            else if (strcmp(key, "text") == 0) config->custom_text = ParseRGB(value);
            else if (strcmp(key, "text_muted") == 0) config->custom_text_muted = ParseRGB(value);
        }
        else if (strcmp(section, "Prayer") == 0) {
            if (strcmp(key, "enabled") == 0) config->prayer_enabled = ParseBool(value);
            else if (strcmp(key, "use_api") == 0) config->prayer_use_api = ParseBool(value);
            else if (strcmp(key, "city") == 0) {
                strncpy(config->prayer_city, value, sizeof(config->prayer_city) - 1);
                config->prayer_city[sizeof(config->prayer_city) - 1] = '\0';
            }
            else if (strcmp(key, "country") == 0) {
                strncpy(config->prayer_country, value, sizeof(config->prayer_country) - 1);
                config->prayer_country[sizeof(config->prayer_country) - 1] = '\0';
            }
            else if (strcmp(key, "method") == 0) {
                config->prayer_method = atoi(value);
                if (config->prayer_method < 0 || config->prayer_method > 15) {
                    config->prayer_method = 2;
                }
            }
            else if (strcmp(key, "fajr") == 0) {
                strncpy(config->prayer_fajr, value, sizeof(config->prayer_fajr) - 1);
                config->prayer_fajr[sizeof(config->prayer_fajr) - 1] = '\0';
            }
            else if (strcmp(key, "dhuhr") == 0) {
                strncpy(config->prayer_dhuhr, value, sizeof(config->prayer_dhuhr) - 1);
                config->prayer_dhuhr[sizeof(config->prayer_dhuhr) - 1] = '\0';
            }
            else if (strcmp(key, "asr") == 0) {
                strncpy(config->prayer_asr, value, sizeof(config->prayer_asr) - 1);
                config->prayer_asr[sizeof(config->prayer_asr) - 1] = '\0';
            }
            else if (strcmp(key, "maghrib") == 0) {
                strncpy(config->prayer_maghrib, value, sizeof(config->prayer_maghrib) - 1);
                config->prayer_maghrib[sizeof(config->prayer_maghrib) - 1] = '\0';
            }
            else if (strcmp(key, "isha") == 0) {
                strncpy(config->prayer_isha, value, sizeof(config->prayer_isha) - 1);
                config->prayer_isha[sizeof(config->prayer_isha) - 1] = '\0';
            }
        }
    }

    fclose(file);
}

/*
 * SaveConfigINI
 * Sauvegarde la configuration dans un fichier INI
 */
void SaveConfigINI(const ConfigINI* config, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "; Performance Overlay Configuration\n");
    fprintf(file, "; Fichier généré automatiquement\n\n");

    fprintf(file, "[Window]\n");
    fprintf(file, "x = %d\n", config->x);
    fprintf(file, "y = %d\n", config->y);
    fprintf(file, "width = %d\n", config->width);
    fprintf(file, "height = %d\n", config->height);
    fprintf(file, "opacity = %d\n", config->opacity);
    fprintf(file, "minimal_mode = %s\n", config->minimal_mode ? "true" : "false");
    fprintf(file, "always_on_top = %s\n\n", config->always_on_top ? "true" : "false");

    fprintf(file, "[Display]\n");
    fprintf(file, "theme = %s\n", config->theme);
    fprintf(file, "font_name = %s\n", config->font_name);
    fprintf(file, "font_size = %d\n", config->font_size);
    fprintf(file, "show_uptime = %s\n", config->show_uptime ? "true" : "false");
    fprintf(file, "show_processes = %s\n", config->show_processes ? "true" : "false");
    fprintf(file, "show_frequency = %s\n\n", config->show_frequency ? "true" : "false");

    fprintf(file, "[Animations]\n");
    fprintf(file, "; Activer/désactiver les animations\n");
    fprintf(file, "enabled = %s\n", config->animations_enabled ? "true" : "false");
    fprintf(file, "; Seuils d'alerte (pulsation quand dépassé)\n");
    fprintf(file, "alert_cpu_threshold = %d\n", config->alert_cpu_threshold);
    fprintf(file, "alert_ram_threshold = %d\n\n", config->alert_ram_threshold);

    fprintf(file, "[Performance]\n");
    fprintf(file, "refresh_interval_ms = %d\n", config->refresh_interval_ms);
    fprintf(file, "max_disks = %d\n\n", config->max_disks);

    fprintf(file, "[Hotkeys]\n");
    fprintf(file, "toggle_visibility = %s\n", VirtualKeyToString(config->toggle_visibility_key));
    fprintf(file, "toggle_minimal = %s\n", VirtualKeyToString(config->toggle_minimal_key));
    fprintf(file, "reload_config = %s\n\n", VirtualKeyToString(config->reload_config_key));

    fprintf(file, "[Metrics]\n");
    fprintf(file, "; Activer/désactiver les métriques\n");
    fprintf(file, "cpu_enabled = %s\n", config->cpu_enabled ? "true" : "false");
    fprintf(file, "ram_enabled = %s\n", config->ram_enabled ? "true" : "false");
    fprintf(file, "disk_enabled = %s\n", config->disk_enabled ? "true" : "false");
    fprintf(file, "uptime_enabled = %s\n", config->uptime_enabled ? "true" : "false");
    fprintf(file, "process_enabled = %s\n\n", config->process_enabled ? "true" : "false");

    fprintf(file, "[Theme]\n");
    fprintf(file, "; Index du thème (0=Neon Dark, 1=Cyberpunk, 2=Matrix, 3=Ocean, 4=Sunset, 5=Custom)\n");
    fprintf(file, "index = %d\n\n", config->theme_index);

    fprintf(file, "[CustomTheme]\n");
    fprintf(file, "; Thème personnalisé (activer avec index = 5)\n");
    fprintf(file, "; Format des couleurs: R,G,B (0-255)\n");
    fprintf(file, "enabled = %s\n", config->custom_theme_enabled ? "true" : "false");
    fprintf(file, "bg = %d,%d,%d\n", GetRValue(config->custom_bg), GetGValue(config->custom_bg), GetBValue(config->custom_bg));
    fprintf(file, "panel = %d,%d,%d\n", GetRValue(config->custom_panel), GetGValue(config->custom_panel), GetBValue(config->custom_panel));
    fprintf(file, "border = %d,%d,%d\n", GetRValue(config->custom_border), GetGValue(config->custom_border), GetBValue(config->custom_border));
    fprintf(file, "accent = %d,%d,%d\n", GetRValue(config->custom_accent), GetGValue(config->custom_accent), GetBValue(config->custom_accent));
    fprintf(file, "accent2 = %d,%d,%d\n", GetRValue(config->custom_accent2), GetGValue(config->custom_accent2), GetBValue(config->custom_accent2));
    fprintf(file, "text = %d,%d,%d\n", GetRValue(config->custom_text), GetGValue(config->custom_text), GetBValue(config->custom_text));
    fprintf(file, "text_muted = %d,%d,%d\n\n", GetRValue(config->custom_text_muted), GetGValue(config->custom_text_muted), GetBValue(config->custom_text_muted));

    fprintf(file, "[Prayer]\n");
    fprintf(file, "; Configuration des horaires de prière\n");
    fprintf(file, "enabled = %s\n", config->prayer_enabled ? "true" : "false");
    fprintf(file, "; use_api = true utilise l'API Aladhan, false = horaires manuels\n");
    fprintf(file, "use_api = %s\n", config->prayer_use_api ? "true" : "false");
    fprintf(file, "city = %s\n", config->prayer_city);
    fprintf(file, "country = %s\n", config->prayer_country);
    fprintf(file, "; Méthodes: 2=ISNA, 3=MWL, 4=Makkah, 5=Egypt, 7=Tehran, 12=UOIF\n");
    fprintf(file, "method = %d\n", config->prayer_method);
    fprintf(file, "; Horaires manuels (utilisés si use_api = false)\n");
    fprintf(file, "fajr = %s\n", config->prayer_fajr);
    fprintf(file, "dhuhr = %s\n", config->prayer_dhuhr);
    fprintf(file, "asr = %s\n", config->prayer_asr);
    fprintf(file, "maghrib = %s\n", config->prayer_maghrib);
    fprintf(file, "isha = %s\n", config->prayer_isha);

    fclose(file);
}
