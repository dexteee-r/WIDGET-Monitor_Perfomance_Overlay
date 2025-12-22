# Structure du Projet - Performance Overlay

## 📁 Arborescence Finale (Clean pour Release)

```
Widget-perf_overlay/
├── 📁 src/                      # Code source
│   ├── main.c                   # Programme principal
│   ├── performance.c            # Monitoring système
│   ├── config.c                 # Gestion config legacy
│   ├── config_parser.c          # Parser INI
│   ├── startup.c                # Démarrage automatique
│   ├── metric_plugin.c          # Système de plugins
│   └── 📁 plugins/              # Plugins de métriques
│       ├── plugin_cpu.c
│       ├── plugin_ram.c
│       ├── plugin_disk.c
│       ├── plugin_uptime.c
│       └── plugin_process.c
│
├── 📁 include/                  # Headers
│   ├── performance.h
│   ├── config.h
│   ├── config_parser.h
│   ├── startup.h
│   ├── constants.h
│   ├── metric_plugin.h
│   └── all_plugins.h
│
├── 📁 doc's/                    # Documentation
│   └── (guides supplémentaires)
│
├── 📁 Screenchots/              # Captures d'écran
│   └── (images du widget)
│
├── 📁 build/                    # Fichiers compilés (ignorés par git)
│   └── *.o
│
├── 📄 icon.ico                  # Icône de l'application
├── 📄 resources.rc              # Ressources Windows
├── 📄 Makefile                  # Build automation
├── 📄 build.bat                 # Build script Windows
├── 📄 .gitignore                # Fichiers ignorés
├── 📄 LICENSE                   # Licence MIT
├── 📄 README.md                 # Documentation principale
├── 📄 CHANGELOG.md              # Historique des versions
├── 📄 ARCHITECTURE.md           # Architecture du code
└── 📄 refresh_icon.bat          # Utilitaire cache icônes

```

## 🗑️ Fichiers à SUPPRIMER pour la release

- ❌ `test_window.c` - Fichier de test temporaire
- ❌ `test_window.exe` - Exécutable de test
- ❌ `compile_test.bat` - Script de test
- ❌ `test_visible.bat` - Script de test
- ❌ `config.txt` - Généré à l'exécution
- ❌ `config.ini` - Généré à l'exécution
- ❌ `PerformanceOverlay_v2_debug.exe` - Version debug
- ❌ `other compilator's/` - Dossier de test compilateurs
- ❌ `build/` - Fichiers compilés (sera regénéré)

## ✅ Fichiers à CONSERVER

### **Code Source**
- ✅ Tous les fichiers `src/` et `include/`
- ✅ `Makefile`, `build.bat`
- ✅ `resources.rc`, `icon.ico`

### **Documentation**
- ✅ `README.md`
- ✅ `LICENSE`
- ✅ `CHANGELOG.md`
- ✅ `ARCHITECTURE.md`
- ✅ Contenu de `doc's/` et `Screenchots/`

### **Configuration**
- ✅ `.gitignore`
- ✅ `refresh_icon.bat` (utilitaire)

## 📦 Pour la Release GitHub

### **Fichiers à inclure dans le .zip**
```
Performance-Overlay-v2.0/
├── PerformanceOverlay_v2.exe    # Exécutable compilé
├── README.md                    # Guide d'utilisation
├── LICENSE                      # Licence
└── icon.ico                     # Pour info
```

### **Release Notes (à créer)**
Voir CHANGELOG.md pour le contenu des release notes.
