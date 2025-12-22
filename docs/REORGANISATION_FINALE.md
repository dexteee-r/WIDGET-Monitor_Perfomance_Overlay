# 📁 Réorganisation Finale - Guide Complet

## 🎯 Objectif

Créer une structure propre, professionnelle et logique pour la release GitHub.

---

## 📋 Structure Finale

```
Widget-perf_overlay/
├── 📁 src/                    # Code source C
│   ├── main.c
│   ├── performance.c
│   ├── config.c
│   ├── startup.c
│   ├── metric_plugin.c
│   ├── config_parser.c
│   └── plugins/
│       ├── plugin_cpu.c
│       ├── plugin_ram.c
│       ├── plugin_disk.c
│       ├── plugin_uptime.c
│       └── plugin_process.c
│
├── 📁 include/                # Headers
│   ├── performance.h
│   ├── config.h
│   ├── constants.h
│   ├── metric_plugin.h
│   ├── all_plugins.h
│   └── ...
│
├── 📁 docs/                   # 📚 TOUTE la documentation
│   ├── ARCHITECTURE.md
│   ├── REFACTORING_GUIDE.md
│   ├── SUMMARY_REFACTORING.md
│   ├── PROJECT_STRUCTURE.md
│   ├── RELEASE_GUIDE.md
│   └── QUICK_RELEASE.md
│
├── 📁 resources/              # 🎨 Ressources (icônes, RC, configs)
│   ├── icon.ico              # Icône de l'application
│   ├── resources.rc          # Ressources Windows
│   ├── config.ini.example    # Exemple de configuration
│   └── README.txt            # Info sur les ressources
│
├── 📁 screenshots/            # 📸 Captures d'écran
│   └── *.png
│
├── 📁 scripts/                # 🔧 Scripts utilitaires
│   ├── build.bat             # Script de build unifié
│   └── refresh_icon.bat      # Nettoyage cache icônes
│
├── 📁 build/                  # Fichiers compilés (dans .gitignore)
│
├── 📄 .gitignore              # Configuration Git
├── 📄 README.md               # Documentation principale
├── 📄 CHANGELOG.md            # Historique des versions
├── 📄 LICENSE                 # Licence MIT
└── 📄 Makefile                # Automatisation de build
```

---

## 🚀 Commandes de Réorganisation

### **Étape 1 : Lancer la réorganisation**

```powershell
.\reorganize_clean.bat
```

**Ce script va :**
1. ✅ Créer `docs/`, `resources/`, `scripts/`, `screenshots/`
2. ✅ Déplacer TOUS les `.md` dans `docs/`
3. ✅ Déplacer `icon.ico`, `resources.rc`, `config.ini` dans `resources/`
4. ✅ Fusionner `build.bat` et `build_release.bat` en un seul script
5. ✅ Déplacer scripts dans `scripts/`
6. ✅ Renommer `Screenchots/` → `screenshots/`
7. ✅ Supprimer fichiers inutiles (`inspi/`, `create_icon.py`, etc.)

---

## 🔧 Utilisation Après Réorganisation

### **Build Normal**

```powershell
.\scripts\build.bat
# OU
make
```

### **Build Release (avec package)**

```powershell
.\scripts\build.bat release
```

Cela crée automatiquement :
```
Performance-Overlay-v2.0/
├── PerformanceOverlay_v2.exe
├── README.md
├── LICENSE
├── CHANGELOG.md
└── icon.ico
```

### **Nettoyage**

```powershell
.\scripts\build.bat clean
# OU
make clean
```

---

## 📦 Création du Package Release

```powershell
# 1. Builder la release
.\scripts\build.bat release

# 2. Créer le ZIP
Compress-Archive -Path "Performance-Overlay-v2.0" -DestinationPath "Performance-Overlay-v2.0.zip" -Force
```

---

## ✅ Vérification Post-Réorganisation

```powershell
# Afficher la structure
tree /F

# Vérifier que les dossiers existent
ls docs/
ls resources/
ls scripts/
ls screenshots/

# Tester la compilation
make clean
make
```

---

## 📝 Fichiers Modifiés

### **Makefile**
- ✅ Ajout de `RES_DIR = resources`
- ✅ Mise à jour du chemin vers `resources/resources.rc`
- ✅ Suppression de la cible `icon` (plus nécessaire)

### **resources.rc**
- ✅ Chemin relatif `icon.ico` (fonctionne car dans le même dossier)

### **.gitignore**
- ✅ Ajout de `config.ini` (généré au runtime)
- ✅ Ajout de patterns pour tests et packages

---

## 🎯 Avantages de Cette Structure

1. **Clarté** : Tout est catégorisé logiquement
2. **Professionnalisme** : Structure standard de projet open-source
3. **Maintenabilité** : Facile de trouver ce qu'on cherche
4. **GitHub-friendly** : Structure optimale pour les releases
5. **Simplicité** : Un seul script de build au lieu de plusieurs

---

## 🚨 Important

Après la réorganisation :

1. ⚠️ Ne pas oublier de supprimer les anciens scripts :
   - `reorganize.bat`
   - `build.bat` (à la racine)
   - `build_release.bat` (à la racine)

2. ✅ Vérifier que le build fonctionne :
   ```powershell
   make clean
   make
   ```

3. ✅ Tester l'exécutable :
   ```powershell
   .\PerformanceOverlay_v2.exe
   ```

---

**Prêt pour une release GitHub propre ! 🚀**
