/*
 * plugin_cpu.c
 * Plugin de monitoring CPU
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/performance.h"
#include "../../include/constants.h"

// Variables locales pour le calcul CPU
static ULARGE_INTEGER lastIdleTime, lastKernelTime, lastUserTime;

/*
 * BuildBarString
 * Construit une barre ASCII (réutilisé du code original)
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
 * GetCPUUsage
 * Calcule l'utilisation CPU
 */
static float GetCPUUsage() {
    FILETIME idleTime, kernelTime, userTime;
    ULARGE_INTEGER idle, kernel, user;
    ULONGLONG idleDiff, kernelDiff, userDiff, totalDiff;

    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return 0.0f;
    }

    memcpy(&idle, &idleTime, sizeof(FILETIME));
    memcpy(&kernel, &kernelTime, sizeof(FILETIME));
    memcpy(&user, &userTime, sizeof(FILETIME));

    idleDiff = idle.QuadPart - lastIdleTime.QuadPart;
    kernelDiff = kernel.QuadPart - lastKernelTime.QuadPart;
    userDiff = user.QuadPart - lastUserTime.QuadPart;

    totalDiff = kernelDiff + userDiff;

    if (totalDiff == 0) {
        return 0.0f;
    }

    float percent = (float)(((totalDiff - idleDiff) * 100.0) / totalDiff);

    lastIdleTime = idle;
    lastKernelTime = kernel;
    lastUserTime = user;

    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    return percent;
}

/*
 * GetCPUFrequency
 * Obtient la fréquence CPU
 */
static float GetCPUFrequency() {
    HKEY hKey;
    DWORD mhz = 0;
    DWORD size = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&mhz, &size);
        RegCloseKey(hKey);
    }

    return (float)(mhz / 1000.0);
}

/*
 * cpu_init
 * Initialisation du plugin CPU
 */
static void cpu_init(void) {
    FILETIME idleTime, kernelTime, userTime;
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
    memcpy(&lastIdleTime, &idleTime, sizeof(FILETIME));
    memcpy(&lastKernelTime, &kernelTime, sizeof(FILETIME));
    memcpy(&lastUserTime, &userTime, sizeof(FILETIME));
}

/*
 * cpu_update
 * Mise à jour des données CPU
 */
static void cpu_update(MetricData* data) {
    float usage = GetCPUUsage();
    float freq = GetCPUFrequency();

    data->value = usage;
    data->line_count = 1;

    // Construire la barre
    char bar[40];
    BuildBarString(bar, sizeof(bar), usage, PROGRESS_BAR_WIDTH);

    // Formater la ligne d'affichage
    snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
             "CPU   %5.1f%%  %4.2f GHz  %s", usage, freq, bar);

    // Couleur selon utilisation
    if (usage > 80.0f) {
        data->color = RGB(255, 100, 100);  // Rouge clair
    } else if (usage > 50.0f) {
        data->color = RGB(255, 200, 100);  // Orange
    } else {
        data->color = COLOR_TEXT_PRIMARY;  // Blanc/Gris
    }
}

/*
 * cpu_cleanup
 * Nettoyage du plugin CPU
 */
static void cpu_cleanup(void) {
    // Rien à nettoyer
}

/*
 * cpu_is_available
 * Vérifier si le CPU monitoring est disponible
 */
static BOOL cpu_is_available(void) {
    return TRUE;  // Toujours disponible
}

// Déclaration du plugin
MetricPlugin CPUPlugin = {
    .plugin_name = "CPU",
    .description = "Monitoring utilisation et fréquence CPU",
    .init = cpu_init,
    .update = cpu_update,
    .cleanup = cpu_cleanup,
    .is_available = cpu_is_available,
    .next = NULL
};
