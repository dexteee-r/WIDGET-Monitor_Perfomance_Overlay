/*
 * plugin_ram.c
 * Plugin de monitoring RAM
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

/*
 * BuildBarString
 * Construit une barre ASCII
 */
static void BuildBarString(char* buffer, size_t len, float percent, int width) {
    if (len == 0 || (int)len <= width + 3) {
        if (len > 0) buffer[0] = '\0';
        return;
    }

    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    int filled = (int)((percent / 100.0f) * width + 0.5f);
    if (filled > width) filled = width;
    int arrowPos = (filled > 0) ? filled - 1 : 0;

    buffer[0] = '[';
    for (int i = 0; i < width; i++) {
        if (i < filled - 1) {
            buffer[1 + i] = '=';
        } else if (i == arrowPos && filled > 0) {
            buffer[1 + i] = '>';
        } else {
            buffer[1 + i] = '.';
        }
    }
    buffer[1 + width] = ']';
    buffer[2 + width] = '\0';
}

/*
 * GetMemoryInfo
 * Obtient les informations mémoire
 */
static void GetMemoryInfo(float* usage_percent, float* used_gb, float* total_gb) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memInfo)) {
        *usage_percent = (float)memInfo.dwMemoryLoad;
        *total_gb = (float)(memInfo.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
        *used_gb = (float)((memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0));
    } else {
        *usage_percent = 0.0f;
        *used_gb = 0.0f;
        *total_gb = 0.0f;
    }
}

/*
 * ram_init
 * Initialisation du plugin RAM
 */
static void ram_init(void) {
    // Rien à initialiser
}

/*
 * ram_update
 * Mise à jour des données RAM
 */
static void ram_update(MetricData* data) {
    float usage_percent, used_gb, total_gb;
    GetMemoryInfo(&usage_percent, &used_gb, &total_gb);

    data->value = usage_percent;
    data->line_count = 1;

    // Construire la barre
    char bar[40];
    BuildBarString(bar, sizeof(bar), usage_percent, PROGRESS_BAR_WIDTH);

    // Formater la ligne d'affichage
    snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
             "RAM   %4.1f/%4.1f GB  %5.1f%%  %s",
             used_gb, total_gb, usage_percent, bar);

    // Couleur selon utilisation
    if (usage_percent > 85.0f) {
        data->color = RGB(255, 100, 100);  // Rouge clair
    } else if (usage_percent > 70.0f) {
        data->color = RGB(255, 200, 100);  // Orange
    } else {
        data->color = COLOR_TEXT_PRIMARY;  // Blanc/Gris
    }
}

/*
 * ram_cleanup
 * Nettoyage du plugin RAM
 */
static void ram_cleanup(void) {
    // Rien à nettoyer
}

/*
 * ram_is_available
 * Vérifier si le RAM monitoring est disponible
 */
static BOOL ram_is_available(void) {
    return TRUE;  // Toujours disponible
}

// Déclaration du plugin
MetricPlugin RAMPlugin = {
    .plugin_name = "RAM",
    .description = "Monitoring utilisation mémoire RAM",
    .init = ram_init,
    .update = ram_update,
    .cleanup = ram_cleanup,
    .is_available = ram_is_available,
    .next = NULL
};
