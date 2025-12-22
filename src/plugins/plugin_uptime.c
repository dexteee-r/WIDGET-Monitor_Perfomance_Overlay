/*
 * plugin_uptime.c
 * Plugin de monitoring du temps d'activité système
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

/*
 * FormatUptime
 * Formate le temps en chaîne lisible
 */
static void FormatUptime(char* buffer, size_t len, DWORD seconds) {
    DWORD days = seconds / 86400;
    DWORD hours = (seconds % 86400) / 3600;
    DWORD minutes = (seconds % 3600) / 60;
    DWORD secs = seconds % 60;

    if (days > 0) {
        snprintf(buffer, len, "UPTIME %lud %02luh %02lum %02lus",
                 (unsigned long)days, (unsigned long)hours,
                 (unsigned long)minutes, (unsigned long)secs);
    } else {
        snprintf(buffer, len, "UPTIME %02luh %02lum %02lus",
                 (unsigned long)hours, (unsigned long)minutes,
                 (unsigned long)secs);
    }
}

/*
 * uptime_init
 * Initialisation du plugin Uptime
 */
static void uptime_init(void) {
    // Rien à initialiser
}

/*
 * uptime_update
 * Mise à jour des données Uptime
 */
static void uptime_update(MetricData* data) {
    DWORD uptime_seconds = (DWORD)(GetTickCount64() / 1000);

    data->value = 0.0f;  // Pas de valeur numérique pour uptime
    data->line_count = 1;

    FormatUptime(data->display_lines[0], sizeof(data->display_lines[0]), uptime_seconds);
    data->color = COLOR_CYAN;  // Couleur cyan pour l'uptime
}

/*
 * uptime_cleanup
 * Nettoyage du plugin Uptime
 */
static void uptime_cleanup(void) {
    // Rien à nettoyer
}

/*
 * uptime_is_available
 * Vérifier si l'Uptime monitoring est disponible
 */
static BOOL uptime_is_available(void) {
    return TRUE;  // Toujours disponible
}

// Déclaration du plugin
MetricPlugin UptimePlugin = {
    .plugin_name = "Uptime",
    .description = "Temps depuis le démarrage système",
    .init = uptime_init,
    .update = uptime_update,
    .cleanup = uptime_cleanup,
    .is_available = uptime_is_available,
    .next = NULL
};
