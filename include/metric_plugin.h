/*
 * metric_plugin.h
 * Système de plugins pour les métriques de performance
 * Permet d'ajouter facilement de nouvelles métriques sans modifier le code existant
 */

#ifndef METRIC_PLUGIN_H
#define METRIC_PLUGIN_H

#include <windows.h>

#define MAX_METRIC_LINES 4  // Nombre maximum de lignes par métrique

// Structure représentant les données d'une métrique
typedef struct {
    char name[32];                         // Nom de la métrique (ex: "CPU", "RAM")
    char display_lines[MAX_METRIC_LINES][256];  // Lignes de texte à afficher
    int line_count;                        // Nombre de lignes utilisées
    float value;                           // Valeur principale (pour barres, 0-100)
    COLORREF color;                        // Couleur d'affichage
    BOOL enabled;                          // Plugin actif ou non
} MetricData;

// Interface plugin
typedef struct MetricPlugin {
    // Métadonnées
    const char* plugin_name;               // Nom unique du plugin
    const char* description;               // Description courte

    // Callbacks (fonctions à implémenter)
    void (*init)(void);                    // Initialisation (appelé au démarrage)
    void (*update)(MetricData* data);      // Mise à jour (appelé à chaque frame)
    void (*cleanup)(void);                 // Nettoyage (appelé à la fermeture)
    BOOL (*is_available)(void);            // Vérifier si le plugin peut fonctionner

    // Chaînage pour liste
    struct MetricPlugin* next;
} MetricPlugin;

// Gestionnaire de plugins
void InitPluginSystem(void);
void RegisterMetricPlugin(MetricPlugin* plugin);
void UpdateAllPlugins(void);
void CleanupPluginSystem(void);

// Récupération de métriques
MetricData* GetMetricByName(const char* name);
MetricPlugin* GetPluginByName(const char* name);
int GetActivePluginCount(void);

// Activer/désactiver un plugin
void EnablePlugin(const char* name, BOOL enable);

#endif // METRIC_PLUGIN_H
