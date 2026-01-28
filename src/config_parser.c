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
                if (config->theme_index < 0 || config->theme_index > 4) {
                    config->theme_index = 0;
                }
            }
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
    fprintf(file, "; Index du thème (0=Neon Dark, 1=Cyberpunk, 2=Matrix, 3=Ocean, 4=Sunset)\n");
    fprintf(file, "index = %d\n\n", config->theme_index);

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
