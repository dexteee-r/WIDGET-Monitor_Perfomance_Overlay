/*
 * performance.h
 * En-tete pour les fonctions de monitoring des performances systeme
 */

#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <windows.h>

#define MAX_DISKS 8

// Structure pour stocker les informations de performance
typedef struct {
    float cpu_usage;                 // Utilisation CPU en pourcentage
    float memory_usage;              // Utilisation RAM en pourcentage
    float disk_usage;                // Utilisation disque (premier disque) en pourcentage
    char disk_name[32];              // Nom du premier disque (ex: "C:")
    int disk_count;                  // Nombre de disques detectes
    char disk_names[MAX_DISKS][8];   // Noms des disques (ex: "C:")
    float disk_usages[MAX_DISKS];    // Utilisation par disque

    // Nouvelles metriques
    float memory_used_gb;            // RAM utilisee en GB
    float memory_total_gb;           // RAM totale en GB
    DWORD uptime_seconds;            // Temps depuis le demarrage en secondes
    DWORD process_count;             // Nombre de processus actifs
    float cpu_frequency_ghz;         // Frequence CPU en GHz
} PerformanceData;

// Fonctions de monitoring
void InitPerformanceMonitoring();
void GetPerformanceData(PerformanceData* data);
void CleanupPerformanceMonitoring();

#endif // PERFORMANCE_H
