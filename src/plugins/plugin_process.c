/*
 * plugin_process.c
 * Plugin de monitoring du nombre de processus
 */

#include <windows.h>
#include <psapi.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

/*
 * GetProcessCount
 * Obtient le nombre de processus actifs
 */
static DWORD GetProcessCount(void) {
    DWORD processes[1024], bytesReturned, processCount;

    if (EnumProcesses(processes, sizeof(processes), &bytesReturned)) {
        processCount = bytesReturned / sizeof(DWORD);
        return processCount;
    }

    return 0;
}

/*
 * process_init
 * Initialisation du plugin Process
 */
static void process_init(void) {
    // Rien à initialiser
}

/*
 * process_update
 * Mise à jour des données Process
 */
static void process_update(MetricData* data) {
    DWORD count = GetProcessCount();

    data->value = (float)count;
    data->line_count = 1;

    snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
             "PROC  %lu", (unsigned long)count);

    data->color = COLOR_TEXT_PRIMARY;
}

/*
 * process_cleanup
 * Nettoyage du plugin Process
 */
static void process_cleanup(void) {
    // Rien à nettoyer
}

/*
 * process_is_available
 * Vérifier si le Process monitoring est disponible
 */
static BOOL process_is_available(void) {
    return TRUE;  // Toujours disponible
}

// Déclaration du plugin
MetricPlugin ProcessPlugin = {
    .plugin_name = "Process",
    .description = "Nombre de processus actifs",
    .init = process_init,
    .update = process_update,
    .cleanup = process_cleanup,
    .is_available = process_is_available,
    .next = NULL
};
