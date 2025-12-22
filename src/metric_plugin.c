/*
 * metric_plugin.c
 * Implémentation du système de plugins pour métriques
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../include/metric_plugin.h"

// Liste chaînée de plugins
static MetricPlugin* g_pluginList = NULL;

// Stockage des données de métriques
#define MAX_METRICS 16
static MetricData g_metricsData[MAX_METRICS];
static int g_metricsCount = 0;

/*
 * InitPluginSystem
 * Initialise le système de plugins
 */
void InitPluginSystem(void) {
    g_pluginList = NULL;
    g_metricsCount = 0;
    memset(g_metricsData, 0, sizeof(g_metricsData));
}

/*
 * RegisterMetricPlugin
 * Enregistre un nouveau plugin dans le système
 */
void RegisterMetricPlugin(MetricPlugin* plugin) {
    if (!plugin) return;

    // Vérifier si le plugin est disponible sur ce système
    if (plugin->is_available && !plugin->is_available()) {
        return;  // Plugin non disponible, ne pas l'enregistrer
    }

    // Ajouter à la liste chaînée
    plugin->next = g_pluginList;
    g_pluginList = plugin;

    // Initialiser le plugin
    if (plugin->init) {
        plugin->init();
    }

    // Créer une entrée de données pour ce plugin
    if (g_metricsCount < MAX_METRICS) {
        MetricData* data = &g_metricsData[g_metricsCount];
        strncpy(data->name, plugin->plugin_name, sizeof(data->name) - 1);
        data->enabled = TRUE;
        data->line_count = 0;
        data->value = 0.0f;
        data->color = RGB(200, 210, 230);  // Couleur par défaut
        g_metricsCount++;
    }
}

/*
 * UpdateAllPlugins
 * Met à jour tous les plugins enregistrés
 */
void UpdateAllPlugins(void) {
    // Parcourir tous les plugins et trouver leurs données correspondantes
    MetricPlugin* plugin = g_pluginList;

    while (plugin) {
        // Chercher les données correspondant à ce plugin
        for (int i = 0; i < g_metricsCount; i++) {
            MetricData* data = &g_metricsData[i];

            if (strcmp(data->name, plugin->plugin_name) == 0 && data->enabled) {
                if (plugin->update) {
                    plugin->update(data);
                }
                break;  // Plugin trouvé, passer au suivant
            }
        }

        plugin = plugin->next;
    }
}

/*
 * CleanupPluginSystem
 * Nettoie tous les plugins
 */
void CleanupPluginSystem(void) {
    MetricPlugin* plugin = g_pluginList;

    while (plugin) {
        if (plugin->cleanup) {
            plugin->cleanup();
        }
        plugin = plugin->next;
    }

    g_pluginList = NULL;
    g_metricsCount = 0;
}

/*
 * GetMetricByName
 * Récupère les données d'une métrique par son nom
 */
MetricData* GetMetricByName(const char* name) {
    if (!name) return NULL;

    for (int i = 0; i < g_metricsCount; i++) {
        if (strcmp(g_metricsData[i].name, name) == 0) {
            return &g_metricsData[i];
        }
    }

    return NULL;
}

/*
 * GetPluginByName
 * Récupère un plugin par son nom
 */
MetricPlugin* GetPluginByName(const char* name) {
    if (!name) return NULL;

    MetricPlugin* plugin = g_pluginList;
    while (plugin) {
        if (strcmp(plugin->plugin_name, name) == 0) {
            return plugin;
        }
        plugin = plugin->next;
    }

    return NULL;
}

/*
 * GetActivePluginCount
 * Retourne le nombre de plugins actifs
 */
int GetActivePluginCount(void) {
    int count = 0;
    for (int i = 0; i < g_metricsCount; i++) {
        if (g_metricsData[i].enabled) {
            count++;
        }
    }
    return count;
}

/*
 * EnablePlugin
 * Active ou désactive un plugin
 */
void EnablePlugin(const char* name, BOOL enable) {
    MetricData* data = GetMetricByName(name);
    if (data) {
        data->enabled = enable;
    }
}
