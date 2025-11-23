/*
 * performance.c
 * Implémentation des fonctions de monitoring système
 */

#include <windows.h>
#include <stdio.h>
#include <psapi.h>
#include "../include/performance.h"

// Variables globales pour le monitoring CPU
static ULARGE_INTEGER lastIdleTime, lastKernelTime, lastUserTime;
static int numProcessors;
static BOOL firstRun = TRUE;

/*
 * InitPerformanceMonitoring
 * Initialise le système de monitoring des performances
 * Appelée une fois au démarrage du programme
 */
void InitPerformanceMonitoring() {
    SYSTEM_INFO sysInfo;
    FILETIME idleTime, kernelTime, userTime;

    // Obtenir le nombre de processeurs
    GetSystemInfo(&sysInfo);
    numProcessors = sysInfo.dwNumberOfProcessors;

    // Initialiser les temps système
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
    memcpy(&lastIdleTime, &idleTime, sizeof(FILETIME));
    memcpy(&lastKernelTime, &kernelTime, sizeof(FILETIME));
    memcpy(&lastUserTime, &userTime, sizeof(FILETIME));
}

/*
 * GetCPUUsage
 * Calcule l'utilisation CPU du système en pourcentage
 * Retourne un nombre entre 0.0 et 100.0
 */
static float GetCPUUsage() {
    FILETIME idleTime, kernelTime, userTime;
    ULARGE_INTEGER idle, kernel, user;
    ULONGLONG idleDiff, kernelDiff, userDiff, totalDiff;
    float percent;

    // Obtenir les temps système actuels
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        return 0.0f;
    }

    memcpy(&idle, &idleTime, sizeof(FILETIME));
    memcpy(&kernel, &kernelTime, sizeof(FILETIME));
    memcpy(&user, &userTime, sizeof(FILETIME));

    // Calculer les différences depuis le dernier appel
    idleDiff = idle.QuadPart - lastIdleTime.QuadPart;
    kernelDiff = kernel.QuadPart - lastKernelTime.QuadPart;
    userDiff = user.QuadPart - lastUserTime.QuadPart;

    // Le temps kernel inclut le temps idle, il faut le soustraire
    totalDiff = kernelDiff + userDiff;

    // Éviter la division par zéro
    if (totalDiff == 0) {
        return 0.0f;
    }

    // Calculer le pourcentage d'utilisation
    // CPU Usage = (Total - Idle) / Total * 100
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
 * Obtient l'utilisation de la mémoire RAM en pourcentage
 */
static float GetMemoryUsage() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);

    // Obtenir les informations mémoire
    GlobalMemoryStatusEx(&memInfo);

    // Retourner le pourcentage utilisé
    return (float)memInfo.dwMemoryLoad;
}

/*
 * GetMemoryDetails
 * Obtient les détails de la mémoire RAM (utilisée et totale en GB)
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
 * Obtient le temps écoulé depuis le démarrage du système en secondes
 */
static DWORD GetSystemUptime() {
    // GetTickCount64 retourne le temps en millisecondes depuis le démarrage
    return (DWORD)(GetTickCount64() / 1000);
}

/*
 * GetProcessCount
 * Obtient le nombre de processus actifs sur le système
 */
static DWORD GetProcessCount() {
    DWORD processes[1024], bytesReturned, processCount;

    // Énumérer tous les processus
    if (EnumProcesses(processes, sizeof(processes), &bytesReturned)) {
        // Calculer le nombre de processus
        processCount = bytesReturned / sizeof(DWORD);
        return processCount;
    }

    return 0;
}

/*
 * GetCPUFrequency
 * Obtient la fréquence du CPU en GHz
 */
static float GetCPUFrequency() {
    HKEY hKey;
    DWORD mhz = 0;
    DWORD size = sizeof(DWORD);

    // Lire la fréquence depuis le registre
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
 * GetDiskUsage
 * Obtient l'utilisation du disque C: en pourcentage
 */
static float GetDiskUsage(char* diskName) {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

    // Copier le nom du disque
    strcpy(diskName, "C:\\");

    // Obtenir les informations du disque
    if (GetDiskFreeSpaceEx("C:\\", &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        // Calculer le pourcentage utilisé
        ULONGLONG usedBytes = totalBytes.QuadPart - totalFreeBytes.QuadPart;
        return (float)((double)usedBytes / (double)totalBytes.QuadPart * 100.0);
    }

    return 0.0f;
}

/*
 * GetPerformanceData
 * Récupère toutes les données de performance
 * Cette fonction remplit la structure PerformanceData
 */
void GetPerformanceData(PerformanceData* data) {
    // Récupérer toutes les métriques de base
    data->cpu_usage = GetCPUUsage();
    data->memory_usage = GetMemoryUsage();
    data->disk_usage = GetDiskUsage(data->disk_name);

    // Récupérer les nouvelles métriques
    GetMemoryDetails(&data->memory_used_gb, &data->memory_total_gb);
    data->uptime_seconds = GetSystemUptime();
    data->process_count = GetProcessCount();
    data->cpu_frequency_ghz = GetCPUFrequency();
}

/*
 * CleanupPerformanceMonitoring
 * Nettoie les ressources utilisées par le monitoring
 * Appelée à la fermeture du programme
 */
void CleanupPerformanceMonitoring() {
    // Rien à nettoyer pour l'instant
    // Cette fonction est prête pour des ajouts futurs
}
