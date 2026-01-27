/*
 * plugin_gpu.c
 * Plugin pour afficher les informations GPU
 *
 * Note: L'accès aux métriques GPU détaillées (usage %, température) nécessite
 * des SDK propriétaires (NVAPI pour NVIDIA, ADL pour AMD).
 * Cette implémentation affiche le nom du GPU depuis le registre Windows.
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

// Informations GPU
static char gpuName[128] = "Unknown GPU";
static BOOL gpuAvailable = FALSE;

/*
 * GetGPUNameFromRegistry
 * Obtient le nom du GPU depuis le registre Windows
 */
static void GetGPUNameFromRegistry(void) {
    HKEY hKey;
    char buffer[256];
    DWORD size = sizeof(buffer);
    DWORD type;

    // Essayer plusieurs chemins de registre pour trouver le GPU
    const char* regPaths[] = {
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0000",
        "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e968-e325-11ce-bfc1-08002be10318}\\0001",
        NULL
    };

    for (int i = 0; regPaths[i] != NULL; i++) {
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPaths[i], 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            // Essayer DriverDesc d'abord
            size = sizeof(buffer);
            if (RegQueryValueExA(hKey, "DriverDesc", NULL, &type, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
                strncpy(gpuName, buffer, sizeof(gpuName) - 1);
                gpuName[sizeof(gpuName) - 1] = '\0';
                gpuAvailable = TRUE;
                RegCloseKey(hKey);
                return;
            }

            // Sinon essayer HardwareInformation.AdapterString
            size = sizeof(buffer);
            if (RegQueryValueExA(hKey, "HardwareInformation.AdapterString", NULL, &type, (LPBYTE)buffer, &size) == ERROR_SUCCESS) {
                strncpy(gpuName, buffer, sizeof(gpuName) - 1);
                gpuName[sizeof(gpuName) - 1] = '\0';
                gpuAvailable = TRUE;
                RegCloseKey(hKey);
                return;
            }

            RegCloseKey(hKey);
        }
    }

    // Fallback: essayer via les Display Adapters
    DISPLAY_DEVICEA dd;
    dd.cb = sizeof(DISPLAY_DEVICEA);

    if (EnumDisplayDevicesA(NULL, 0, &dd, 0)) {
        if (strlen(dd.DeviceString) > 0) {
            strncpy(gpuName, dd.DeviceString, sizeof(gpuName) - 1);
            gpuName[sizeof(gpuName) - 1] = '\0';
            gpuAvailable = TRUE;
        }
    }
}

/*
 * gpu_init
 * Initialisation du plugin GPU
 */
static void gpu_init(void) {
    GetGPUNameFromRegistry();
}

/*
 * gpu_update
 * Mise à jour des données GPU
 */
static void gpu_update(MetricData* data) {
    data->value = 0;  // Pas de valeur numérique disponible sans SDK
    data->line_count = 1;

    // Tronquer le nom si trop long pour l'affichage
    char shortName[40];
    strncpy(shortName, gpuName, 36);
    shortName[36] = '\0';

    // Retirer les espaces en fin
    int len = (int)strlen(shortName);
    while (len > 0 && shortName[len-1] == ' ') {
        shortName[--len] = '\0';
    }

    snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
             "GPU   %s", shortName);

    data->color = COLOR_TEXT_PRIMARY;
}

/*
 * gpu_cleanup
 * Nettoyage du plugin GPU
 */
static void gpu_cleanup(void) {
    // Rien à nettoyer
}

/*
 * gpu_is_available
 * Vérifie si le GPU est disponible
 */
static BOOL gpu_is_available(void) {
    return gpuAvailable;
}

// Déclaration du plugin
MetricPlugin GPUPlugin = {
    .plugin_name = "GPU",
    .description = "Affiche le nom du GPU",
    .init = gpu_init,
    .update = gpu_update,
    .cleanup = gpu_cleanup,
    .is_available = gpu_is_available,
    .next = NULL
};
