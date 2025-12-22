/*
 * plugin_disk.c
 * Plugin de monitoring des disques
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

/*
 * GetDiskUsages
 * Détecte les disques et calcule leur utilisation
 */
static int GetDiskUsages(char diskNames[][8], float diskUsages[], int maxDisks) {
    DWORD drives = GetLogicalDrives();
    int count = 0;

    for (int i = 0; i < 26 && count < maxDisks; i++) {
        if (drives & (1 << i)) {
            char rootPath[4] = { (char)('A' + i), ':', '\\', '\0' };
            UINT type = GetDriveType(rootPath);

            if (type == DRIVE_FIXED || type == DRIVE_RAMDISK) {
                ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;
                if (GetDiskFreeSpaceEx(rootPath, &freeBytesAvailable, &totalBytes, &totalFreeBytes) &&
                    totalBytes.QuadPart > 0) {
                    ULONGLONG usedBytes = totalBytes.QuadPart - totalFreeBytes.QuadPart;
                    float usage = (float)((double)usedBytes / (double)totalBytes.QuadPart * 100.0);

                    diskNames[count][0] = rootPath[0];
                    diskNames[count][1] = ':';
                    diskNames[count][2] = '\0';
                    diskUsages[count] = usage;
                    count++;
                }
            }
        }
    }

    return count;
}

/*
 * disk_init
 * Initialisation du plugin Disk
 */
static void disk_init(void) {
    // Rien à initialiser
}

/*
 * disk_update
 * Mise à jour des données Disk
 */
static void disk_update(MetricData* data) {
    char diskNames[MAX_DISKS][8];
    float diskUsages[MAX_DISKS];
    int diskCount = GetDiskUsages(diskNames, diskUsages, MAX_DISKS);

    data->value = (diskCount > 0) ? diskUsages[0] : 0.0f;
    data->line_count = 1;

    // Construire la ligne avec tous les disques
    char line[256] = "DISKS ";
    int written = (int)strlen(line);

    if (diskCount == 0) {
        snprintf(data->display_lines[0], sizeof(data->display_lines[0]), "DISKS n/a");
        data->color = COLOR_TEXT_MUTED;
        return;
    }

    for (int i = 0; i < diskCount; i++) {
        int remaining = (int)sizeof(line) - written;
        if (remaining <= 0) break;

        int added = snprintf(line + written, remaining, "%s %.0f%%",
                             diskNames[i], diskUsages[i]);
        if (added < 0 || added >= remaining) {
            break;
        }
        written += added;

        if (i < diskCount - 1 && written < (int)sizeof(line) - 3) {
            line[written++] = ' ';
            line[written++] = '|';
            line[written++] = ' ';
            line[written] = '\0';
        }
    }

    strncpy(data->display_lines[0], line, sizeof(data->display_lines[0]) - 1);
    data->display_lines[0][sizeof(data->display_lines[0]) - 1] = '\0';
    data->color = COLOR_CYAN;  // Couleur cyan pour les disques
}

/*
 * disk_cleanup
 * Nettoyage du plugin Disk
 */
static void disk_cleanup(void) {
    // Rien à nettoyer
}

/*
 * disk_is_available
 * Vérifier si le Disk monitoring est disponible
 */
static BOOL disk_is_available(void) {
    return TRUE;  // Toujours disponible
}

// Déclaration du plugin
MetricPlugin DiskPlugin = {
    .plugin_name = "Disk",
    .description = "Monitoring utilisation des disques",
    .init = disk_init,
    .update = disk_update,
    .cleanup = disk_cleanup,
    .is_available = disk_is_available,
    .next = NULL
};
