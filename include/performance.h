/*
 * performance.h
 * En-tête pour les fonctions de monitoring des performances système
 */

#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <windows.h>

// Structure pour stocker les informations de performance
typedef struct {
    float cpu_usage;            // Utilisation CPU en pourcentage
    float memory_usage;         // Utilisation RAM en pourcentage
    float disk_usage;           // Utilisation disque en pourcentage
    char disk_name[32];         // Nom du disque (ex: "C:")

    // Nouvelles métriques
    float memory_used_gb;       // RAM utilisée en GB
    float memory_total_gb;      // RAM totale en GB
    DWORD uptime_seconds;       // Temps depuis le démarrage en secondes
    DWORD process_count;        // Nombre de processus actifs
    float cpu_frequency_ghz;    // Fréquence CPU en GHz
} PerformanceData;

// Fonctions de monitoring
void InitPerformanceMonitoring();
void GetPerformanceData(PerformanceData* data);
void CleanupPerformanceMonitoring();

#endif // PERFORMANCE_H
