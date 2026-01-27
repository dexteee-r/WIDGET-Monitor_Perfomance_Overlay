/*
 * all_plugins.h
 * Déclarations de tous les plugins disponibles
 */

#ifndef ALL_PLUGINS_H
#define ALL_PLUGINS_H

#include "metric_plugin.h"

// Déclarations externes des plugins
extern MetricPlugin CPUPlugin;
extern MetricPlugin RAMPlugin;
extern MetricPlugin DiskPlugin;
extern MetricPlugin UptimePlugin;
extern MetricPlugin ProcessPlugin;
extern MetricPlugin NetworkPlugin;
extern MetricPlugin DateTimePlugin;
extern MetricPlugin VolumePlugin;
extern MetricPlugin GPUPlugin;

#endif // ALL_PLUGINS_H
