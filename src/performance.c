/*
 * performance.c
 * Implementation des fonctions de monitoring systeme
 */

#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include <string.h>
#include "../include/performance.h"

// Variables globales pour le monitoring CPU
static ULARGE_INTEGER lastIdleTime, lastKernelTime, lastUserTime;
static int numProcessors;

/*
 * InitPerformanceMonitoring
 * Initialise le systeme de monitoring des performances
 * Appelee une fois au demarrage du programme
 */
void InitPerformanceMonitoring() {
    SYSTEM_INFO sysInfo;
    FILETIME idleTime, kernelTime, userTime;

    // Obtenir le nombre de processeurs
    GetSystemInfo(&sysInfo);
    numProcessors = sysInfo.dwNumberOfProcessors;

    // Initialiser les temps systeme
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
    memcpy(&lastIdleTime, &idleTime, sizeof(FILETIME));
    memcpy(&lastKernelTime, &kernelTime, sizeof(FILETIME));
    memcpy(&lastUserTime, &userTime, sizeof(FILETIME));
}

/*
 * GetCPUUsage
 * Calcule l'utilisation CPU du systeme en pourcentage
 * Retourne un nombre entre 0.0 et 100.0
 */
static float GetCPUUsage() {
    FILETIME idleTime, kernelTime, userTime;
    ULARGE_INTEGER idle, kernel, user;
    ULONGLONG idleDiff, kernelDiff, userDiff, totalDiff;
    float percent;

    // Obtenir les temps systeme actuels
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return 0.0f;
    }

    memcpy(&idle, &idleTime, sizeof(FILETIME));
    memcpy(&kernel, &kernelTime, sizeof(FILETIME));
    memcpy(&user, &userTime, sizeof(FILETIME));

    // Calculer les differences depuis le dernier appel
    idleDiff = idle.QuadPart - lastIdleTime.QuadPart;
    kernelDiff = kernel.QuadPart - lastKernelTime.QuadPart;
    userDiff = user.QuadPart - lastUserTime.QuadPart;

    // Le temps kernel inclut le temps idle, il faut le soustraire
    totalDiff = kernelDiff + userDiff;

    // Eviter la division par zero
    if (totalDiff == 0) {
        return 0.0f;
    }

    // Calculer le pourcentage d'utilisation
    percent = (float)(((totalDiff - idleDiff) * 100.0) / totalDiff);

    // Sauvegarder pour le prochain appel
    lastIdleTime = idle;
    lastKernelTime = kernel;
    lastUserTime = user;

    // Limiter entre 0 et 100
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    return percent;
}

/*
 * GetMemoryUsage
 * Obtient l'utilisation de la memoire RAM en pourcentage
 */
static float GetMemoryUsage() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    // Obtenir les informations memoire
    GlobalMemoryStatusEx(&memInfo);

    // Retourner le pourcentage utilise
    return (float)memInfo.dwMemoryLoad;
}

/*
 * GetMemoryDetails
 * Obtient les details de la memoire RAM (utilisee et totale en GB)
 */
static void GetMemoryDetails(float* used_gb, float* total_gb) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memInfo)) {
        // Convertir les octets en GB (1 GB = 1024^3 octets)
        *total_gb = (float)(memInfo.ullTotalPhys / (1024.0 * 1024.0 * 1024.0));
        *used_gb = (float)((memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0));
    } else {
        *used_gb = 0.0f;
        *total_gb = 0.0f;
    }
}

/*
 * GetSystemUptime
 * Obtient le temps ecoule depuis le demarrage du systeme en secondes
 */
static DWORD GetSystemUptime() {
    // GetTickCount64 retourne le temps en millisecondes depuis le demarrage
    return (DWORD)(GetTickCount64() / 1000);
}

/*
 * GetProcessCount
 * Obtient le nombre de processus actifs sur le systeme
 */
static DWORD GetProcessCount() {
    DWORD processes[1024], bytesReturned, processCount;

    // Enumerer tous les processus
    if (EnumProcesses(processes, sizeof(processes), &bytesReturned)) {
        // Calculer le nombre de processus
        processCount = bytesReturned / sizeof(DWORD);
        return processCount;
    }

    return 0;
}

/*
 * GetCPUFrequency
 * Obtient la frequence du CPU en GHz
 */
static float GetCPUFrequency() {
    HKEY hKey;
    DWORD mhz = 0;
    DWORD size = sizeof(DWORD);

    // Lire la frequence depuis le registre
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                     0,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&mhz, &size);
        RegCloseKey(hKey);
    }

    // Convertir MHz en GHz
    return (float)(mhz / 1000.0);
}

/*
 * GetDiskUsages
 * Detecte les disques fixes et calcule leur taux d'utilisation
 * Remplit les tableaux de noms et pourcentages, retourne le nombre de disques
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
 * GetPerformanceData
 * Recupere toutes les donnees de performance
 * Cette fonction remplit la structure PerformanceData
 */
void GetPerformanceData(PerformanceData* data) {
    // Recuperer toutes les metriques de base
    data->cpu_usage = GetCPUUsage();
    data->memory_usage = GetMemoryUsage();
    data->disk_count = GetDiskUsages(data->disk_names, data->disk_usages, MAX_DISKS);
    if (data->disk_count > 0) {
        data->disk_usage = data->disk_usages[0];
        strcpy(data->disk_name, data->disk_names[0]);
    } else {
        data->disk_usage = 0.0f;
        strcpy(data->disk_name, "N/A");
    }

    // Recuperer les autres metriques
    GetMemoryDetails(&data->memory_used_gb, &data->memory_total_gb);
    data->uptime_seconds = GetSystemUptime();
    data->process_count = GetProcessCount();
    data->cpu_frequency_ghz = GetCPUFrequency();
}

/*
 * CleanupPerformanceMonitoring
 * Nettoie les ressources utilisees par le monitoring
 * Appelee a la fermeture du programme
 */
void CleanupPerformanceMonitoring() {
    // Rien a nettoyer pour l'instant
}
