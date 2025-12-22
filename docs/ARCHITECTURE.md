# Architecture v3.0 - Performance Overlay

## 📐 Vue d'Ensemble

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│                       (main.c)                           │
└────────────────┬────────────────────────────────────────┘
                 │
        ┌────────┴────────┐
        │                 │
┌───────▼───────┐  ┌─────▼──────────┐
│ Config System │  │ Plugin System  │
│ (config_parser)│  │(metric_plugin) │
└───────────────┘  └────────┬───────┘
                            │
                ┌───────────┴───────────┐
                │                       │
        ┌───────▼─────┐        ┌───────▼─────┐
        │   Plugins   │        │  Constants  │
        │  (5 actifs) │        │(constants.h)│
        └─────────────┘        └─────────────┘
```

---

## 🏗️ Composants Principaux

### 1. **Application Layer** (`src/main.c`)
**Responsabilités:**
- Gestion de la fenêtre Win32
- Boucle d'événements
- Rendu GDI
- Gestion des entrées utilisateur

**Dépendances:**
- `metric_plugin` - Pour récupérer les métriques
- `config_parser` - Pour charger/sauvegarder config
- `constants` - Pour les valeurs constantes

---

### 2. **Plugin System** (`src/metric_plugin.c`)
**Responsabilités:**
- Enregistrement des plugins
- Mise à jour périodique
- Gestion du cycle de vie (init/update/cleanup)
- Activation/désactivation dynamique

**Interface:**
```c
typedef struct MetricPlugin {
    const char* plugin_name;
    void (*init)(void);
    void (*update)(MetricData* data);
    void (*cleanup)(void);
    BOOL (*is_available)(void);
    struct MetricPlugin* next;
} MetricPlugin;
```

**API Publique:**
```c
void InitPluginSystem(void);
void RegisterMetricPlugin(MetricPlugin* plugin);
void UpdateAllPlugins(void);
MetricData* GetMetricByName(const char* name);
void EnablePlugin(const char* name, BOOL enable);
void CleanupPluginSystem(void);
```

---

### 3. **Plugins** (`src/plugins/*.c`)

Chaque plugin implémente l'interface `MetricPlugin`:

#### **plugin_cpu.c**
- Calcule utilisation CPU (%)
- Lit fréquence CPU (GHz)
- Génère barre de progression ASCII

#### **plugin_ram.c**
- Utilisation RAM (% et GB)
- Mémoire disponible/totale
- Barre de progression

#### **plugin_disk.c**
- Détecte tous les disques fixes
- Calcule utilisation par disque
- Affichage multi-disques (C: 45% | D: 78%)

#### **plugin_uptime.c**
- Temps depuis démarrage
- Format: `12h 34m 56s` ou `2d 12h 34m 56s`

#### **plugin_process.c**
- Nombre de processus actifs
- Via EnumProcesses()

---

### 4. **Configuration System** (`src/config_parser.c`)
**Responsabilités:**
- Parser fichier INI
- Sauvegarder configuration
- Valeurs par défaut
- Conversion de types (bool, VK keys)

**Structure:**
```c
typedef struct {
    // [Window]
    int x, y, width, height, opacity;
    BOOL minimal_mode, always_on_top;

    // [Display]
    char theme[32], font_name[32];
    int font_size;
    BOOL show_uptime, show_processes, show_frequency;

    // [Performance]
    int refresh_interval_ms, max_disks;

    // [Hotkeys]
    int toggle_visibility_key, toggle_minimal_key, reload_config_key;

    // [Metrics]
    BOOL cpu_enabled, ram_enabled, disk_enabled, uptime_enabled, process_enabled;
} ConfigINI;
```

**API:**
```c
void LoadConfigINI(ConfigINI* config, const char* filename);
void SaveConfigINI(const ConfigINI* config, const char* filename);
void SetDefaultConfigINI(ConfigINI* config);
```

---

### 5. **Constants** (`include/constants.h`)
**Contenu:**
- Dimensions fenêtre
- Palette de couleurs
- Polices (nom, taille)
- Timers
- Limites système

**Exemple:**
```c
#define WINDOW_WIDTH 340
#define COLOR_CYAN RGB(0, 230, 255)
#define FONT_NORMAL_NAME "Consolas"
#define TIMER_INTERVAL 2000
```

---

## 🔄 Flux de Données

```
┌──────────────┐
│   Startup    │
└──────┬───────┘
       │
       ├─► InitPluginSystem()
       ├─► RegisterMetricPlugin(&CPUPlugin)
       ├─► RegisterMetricPlugin(&RAMPlugin)
       ├─► ...
       └─► LoadConfigINI()
              │
              ▼
       ┌──────────────┐
       │  Main Loop   │
       └──────┬───────┘
              │
         ┌────┴──── WM_TIMER (every 2s)
         │
         ├─► UpdateAllPlugins()
         │     ├─► plugin_cpu.update()
         │     ├─► plugin_ram.update()
         │     ├─► plugin_disk.update()
         │     ├─► plugin_uptime.update()
         │     └─► plugin_process.update()
         │
         └─► WM_PAINT
               ├─► GetMetricByName("CPU")
               ├─► GetMetricByName("RAM")
               ├─► ...
               └─► Render à l'écran
```

---

## 📦 Structure des Fichiers

```
Widget-perf_overlay/
│
├── include/                    Headers
│   ├── all_plugins.h          Déclarations plugins
│   ├── config.h               Config (ancienne, v2.0)
│   ├── config_parser.h        Config INI (v3.0)
│   ├── constants.h            Constantes centralisées
│   ├── metric_plugin.h        Interface plugins
│   ├── performance.h          API monitoring (v2.0)
│   └── startup.h              Démarrage automatique
│
├── src/                        Sources
│   ├── main.c                 Programme principal
│   ├── config.c               Config v2.0 (legacy)
│   ├── config_parser.c        Parseur INI v3.0
│   ├── metric_plugin.c        Gestionnaire plugins
│   ├── performance.c          Monitoring v2.0 (legacy)
│   ├── startup.c              Registry Windows
│   │
│   └── plugins/               Plugins métriques
│       ├── plugin_cpu.c
│       ├── plugin_ram.c
│       ├── plugin_disk.c
│       ├── plugin_uptime.c
│       └── plugin_process.c
│
├── build/                      Fichiers objets (.o)
├── resources.rc                Ressources Windows
├── icon.ico                    Icône application
├── config.ini                  Configuration utilisateur
│
├── Makefile                    Build Linux/MSYS2
├── build.bat                   Build Windows
│
└── doc's/                      Documentation
    ├── REFACTORING_GUIDE.md   Guide technique
    ├── CHANGELOG_V3.md        Changelog v3.0
    └── ARCHITECTURE.md        Ce fichier
```

---

## 🔌 Ajouter un Nouveau Plugin

### Étape 1: Créer le fichier plugin

**`src/plugins/plugin_network.c`:**
```c
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

static void network_init(void) {
    // Initialisation
}

static void network_update(MetricData* data) {
    // Récupérer vitesse download/upload
    float download_mbps = GetDownloadSpeed();
    float upload_mbps = GetUploadSpeed();

    data->value = download_mbps;
    data->line_count = 1;

    snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
             "NET   ↓%.1f Mbps  ↑%.1f Mbps", download_mbps, upload_mbps);
    data->color = COLOR_CYAN;
}

static void network_cleanup(void) {
    // Nettoyage
}

static BOOL network_is_available(void) {
    return TRUE;
}

MetricPlugin NetworkPlugin = {
    .plugin_name = "Network",
    .description = "Monitoring réseau",
    .init = network_init,
    .update = network_update,
    .cleanup = network_cleanup,
    .is_available = network_is_available,
    .next = NULL
};
```

### Étape 2: Déclarer dans `include/all_plugins.h`
```c
extern MetricPlugin NetworkPlugin;
```

### Étape 3: Ajouter au Makefile/build.bat
```makefile
SOURCES += $(PLUGIN_DIR)/plugin_network.c
OBJECTS += $(BUILD_DIR)/plugin_network.o
```

### Étape 4: Enregistrer dans main.c
```c
RegisterMetricPlugin(&NetworkPlugin);
```

**C'est tout !** ✅ Pas besoin de modifier autre chose.

---

## 🎨 Personnalisation

### Changer les couleurs
**Éditer `include/constants.h`:**
```c
#define COLOR_CYAN RGB(0, 255, 128)      // Vert néon
#define COLOR_MAGENTA RGB(255, 0, 128)   // Rose néon
```

### Changer la police
**Éditer `config.ini`:**
```ini
[Display]
font_name = Courier New
font_size = 14
```

### Changer l'intervalle de mise à jour
**Éditer `config.ini`:**
```ini
[Performance]
refresh_interval_ms = 1000  # 1 seconde au lieu de 2
```

---

## 🧪 Tests

### Tester un plugin isolé
```c
// Dans un main() de test
MetricData data;
CPUPlugin.init();
CPUPlugin.update(&data);
printf("%s\n", data.display_lines[0]);
CPUPlugin.cleanup();
```

### Tester le parseur INI
```c
ConfigINI config;
LoadConfigINI(&config, "test.ini");
printf("Position: %d, %d\n", config.x, config.y);
SaveConfigINI(&config, "output.ini");
```

---

## 📈 Performances

| Composant | RAM | CPU (idle) | CPU (update) |
|-----------|-----|-----------|--------------|
| **Application** | ~2-3 MB | < 0.5% | < 1% |
| **Plugin System** | +200 KB | +0.1% | +0.2% |
| **Config Parser** | +50 KB | 0% | 0% |
| **5 Plugins** | +300 KB | +0.2% | +0.5% |
| **TOTAL v3.0** | ~3 MB | < 1% | ~2% |

**Conclusion:** Overhead négligeable par rapport à v2.0.

---

## 🔮 Évolutions Futures

### v3.1 - Métriques Avancées
- Plugin GPU (NVIDIA/AMD)
- Plugin Température (CPU/GPU)
- Plugin Réseau (download/upload)
- Plugin Batterie (laptops)

### v3.2 - Interface
- Graphiques temps réel (lignes/barres)
- Historique 60s par métrique
- Menu contextuel (clic droit)
- Thèmes chargeables (INI)

### v3.3 - Système
- Multi-fenêtres (plusieurs overlays)
- Support multi-écrans
- Scripting (Lua/Python)
- API REST (contrôle externe)

---

## 📚 Références

- **Win32 API**: [Microsoft Docs](https://docs.microsoft.com/en-us/windows/win32/)
- **GDI Graphics**: [MSDN GDI](https://docs.microsoft.com/en-us/windows/win32/gdi/)
- **Plugin Architecture**: Design pattern Strategy/Observer
- **INI Parser**: Format standard Windows

---

**Version:** 3.0
**Date:** 2025-12-19
**Auteur:** Refactoring Team
