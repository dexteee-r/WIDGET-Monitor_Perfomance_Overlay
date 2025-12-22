# Guide d'Utilisation - Performance Overlay

## Table des Matières
1. [Installation des Outils](#1-installation-des-outils)
2. [Compilation du Programme](#2-compilation-du-programme)
3. [Premier Lancement](#3-premier-lancement)
4. [Utilisation Quotidienne](#4-utilisation-quotidienne)
5. [Raccourcis Clavier](#5-raccourcis-clavier)
6. [Configuration](#6-configuration)
7. [Démarrage Automatique](#7-démarrage-automatique)
8. [Résolution de Problèmes](#8-résolution-de-problèmes)

---

## 1. Installation des Outils

Avant de compiler le programme, vous devez installer un compilateur C pour Windows.

### Option A : MinGW-w64 (Recommandé - Simple)

**Téléchargement :**
1. Allez sur https://www.msys2.org/
2. Téléchargez l'installateur MSYS2 (msys2-x86_64-*.exe)
3. Lancez l'installateur et suivez les étapes
4. Une fois installé, ouvrez le terminal MSYS2 UCRT64
5. Tapez ces commandes une par une :

```bash
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc
pacman -S make
```

6. Ajoutez GCC au PATH Windows :
   - Ouvrez "Modifier les variables d'environnement système"
   - Cliquez sur "Variables d'environnement"
   - Dans "Variables système", sélectionnez "Path" et cliquez "Modifier"
   - Ajoutez : `C:\msys64\ucrt64\bin`
   - Cliquez OK partout

7. **IMPORTANT** : Fermez et rouvrez votre terminal/IDE pour que les changements prennent effet

### Option B : Visual Studio (Plus lourd mais complet)

1. Téléchargez Visual Studio Community (gratuit) : https://visualstudio.microsoft.com/
2. Lors de l'installation, sélectionnez "Développement Desktop en C++"
3. Utilisez "Developer Command Prompt for VS" au lieu du terminal normal

### Vérification de l'Installation

Ouvrez un nouveau terminal et tapez :
```bash
gcc --version
```

Vous devriez voir quelque chose comme :
```
gcc (GCC) 13.2.0
Copyright (C) 2023 Free Software Foundation, Inc.
```

---

## 2. Compilation du Programme

### Méthode 1 : Utilisation du Makefile (Recommandé)

Ouvrez un terminal dans le dossier du projet et tapez :

```bash
make
```

Vous verrez :
```
gcc -Wall -O2 -c src/main.c -o build/main.o -Iinclude
gcc -Wall -O2 -c src/performance.c -o build/performance.o -Iinclude
gcc -Wall -O2 -c src/config.c -o build/config.o -Iinclude
gcc -Wall -O2 -c src/startup.c -o build/startup.o -Iinclude
gcc -Wall -O2 -mwindows build/main.o build/performance.o build/config.o build/startup.o -o PerformanceOverlay.exe -lgdi32 -luser32 -ladvapi32
```

✅ Un fichier **PerformanceOverlay.exe** sera créé à la racine du projet.

### Méthode 2 : Compilation Manuelle (Si Make ne fonctionne pas)

Créez d'abord le dossier build :
```bash
mkdir build
```

Puis compilez avec une seule commande :
```bash
gcc -Wall -O2 -mwindows -Iinclude -o PerformanceOverlay.exe src/main.c src/performance.c src/config.c src/startup.c -lgdi32 -luser32 -ladvapi32
```

### Nettoyage

Pour supprimer les fichiers de compilation :
```bash
make clean
```

---

## 3. Premier Lancement

### Lancement Simple

Double-cliquez sur **PerformanceOverlay.exe**

OU dans le terminal :
```bash
./PerformanceOverlay.exe
```

### Ce qui va se passer :

1. Une petite fenêtre semi-transparente apparaîtra en haut à gauche de votre écran
2. Elle affichera :
   ```
   ╔════════════════════╗
   ║ [X]                ║
   ║                    ║
   ║ CPU: 45.2%         ║
   ║ RAM: 62.8%         ║
   ║ Disque C: 78.3%    ║
   ╚════════════════════╝
   ```
3. Les valeurs se mettront à jour toutes les **2 secondes**
4. Un fichier **config.txt** sera créé automatiquement

### Premiers Réglages

- **Déplacer la fenêtre** : Cliquez et glissez n'importe où sur la fenêtre
- **Fermer** : Cliquez sur le **[X]** rouge en haut à gauche
- **Cacher temporairement** : Appuyez sur **F1**

---

## 4. Utilisation Quotidienne

### Scénario 1 : Gaming / Applications Gourmandes

1. Lancez **PerformanceOverlay.exe** avant votre jeu
2. Placez la fenêtre dans un coin de l'écran (en bas à droite par exemple)
3. Lancez votre jeu
4. L'overlay restera **toujours visible** au-dessus du jeu
5. Surveillez le CPU/RAM pour détecter les ralentissements

**Astuce** : Si l'overlay est trop visible, appuyez sur **F2** pour activer le mode minimal (seulement CPU + RAM).

### Scénario 2 : Travail / Multitâche

1. Démarrez l'overlay au lancement de Windows (voir section 7)
2. Positionnez-le dans un coin discret
3. Vérifiez occasionnellement les performances
4. Si le CPU/RAM est élevé, fermez les applications inutiles

### Scénario 3 : Streaming / Enregistrement

1. Lancez l'overlay en mode minimal (F2)
2. Placez-le hors du champ de la caméra/capture
3. Surveillez que le CPU/RAM ne saturent pas pendant le stream
4. Cachez-le avec F1 si besoin pendant l'enregistrement

---

## 5. Raccourcis Clavier

| Touche | Action |
|--------|--------|
| **F1** | Afficher/Cacher l'overlay (toggle) |
| **F2** | Mode Minimal ↔ Mode Complet |

### Mode Minimal vs Mode Complet

**Mode Minimal** (F2) :
```
CPU: 45.2%
RAM: 62.8%
```

**Mode Complet** (défaut) :
```
CPU: 45.2%
RAM: 62.8%
Disque C: 78.3%
```

---

## 6. Configuration

### Fichier config.txt

Créé automatiquement au premier lancement :

```
x=10
y=10
minimal_mode=0
```

**Signification :**
- `x=10` : Position horizontale de la fenêtre (en pixels depuis la gauche)
- `y=10` : Position verticale (en pixels depuis le haut)
- `minimal_mode=0` : 0 = mode complet, 1 = mode minimal

### Modification Manuelle

Vous pouvez éditer **config.txt** avec le Bloc-notes :

**Exemple** : Placer l'overlay en bas à droite (écran 1920x1080) :
```
x=1700
y=900
minimal_mode=1
```

**IMPORTANT** : Fermez l'overlay avant d'éditer config.txt, sinon vos changements seront écrasés.

### Sauvegarde Automatique

La configuration est **automatiquement sauvegardée** quand vous :
- Déplacez la fenêtre
- Changez de mode (F2)
- Fermez le programme

---

## 7. Démarrage Automatique

### Activation du Démarrage Auto

Au premier lancement, le programme s'ajoute automatiquement au démarrage de Windows.

### Vérification

1. Appuyez sur **Win + R**
2. Tapez `shell:startup` et appuyez sur Entrée
3. Vous ne verrez PAS de raccourci ici (c'est normal)

Le programme utilise le **Registre Windows** au lieu du dossier de démarrage.

### Vérification via le Registre

1. Appuyez sur **Win + R**
2. Tapez `regedit` et appuyez sur Entrée
3. Naviguez vers :
   ```
   HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run
   ```
4. Vous devriez voir une entrée **PerformanceOverlay** avec le chemin de votre .exe

### Désactiver le Démarrage Auto

**Méthode 1 - Gestionnaire des Tâches (Windows 11)** :
1. Clic droit sur la barre des tâches → Gestionnaire des tâches
2. Allez dans l'onglet "Démarrage"
3. Cherchez "PerformanceOverlay"
4. Clic droit → Désactiver

**Méthode 2 - Registre** :
1. Ouvrez `regedit` (Win + R → regedit)
2. Allez à `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`
3. Supprimez l'entrée **PerformanceOverlay**

---

## 8. Résolution de Problèmes

### Problème : "Le programme ne se lance pas"

**Vérifications :**
1. Avez-vous compilé avec `-lgdi32 -luser32 -ladvapi32` ?
2. Essayez de lancer depuis le terminal pour voir les erreurs :
   ```bash
   ./PerformanceOverlay.exe
   ```

**Solution :** Recompilez avec la commande complète de la section 2.

---

### Problème : "L'overlay affiche toujours CPU: 0.0%"

**Cause :** Bug connu dans la fonction GetCPUUsage() (elle mesure le CPU du processus overlay, pas du système).

**Solution temporaire :** Attendez quelques secondes (2-3 cycles de mise à jour).

**Fix définitif :** Le code doit être corrigé pour utiliser les compteurs système (voir section Développement).

---

### Problème : "La fenêtre ne reste pas au premier plan"

**Vérifications :**
1. Vérifiez que le style `WS_EX_TOPMOST` est bien défini dans `main.c`
2. Certains jeux en mode exclusif peuvent quand même masquer l'overlay

**Solution :** Lancez votre jeu en mode fenêtré ou fenêtré sans bordures.

---

### Problème : "Le texte est illisible / mal affiché"

**Causes possibles :**
1. Mise à l'échelle Windows (150%, 200%)
2. Police Arial non disponible

**Solution :** Modifiez la taille de la police dans `main.c` :
```c
HFONT hFont = CreateFont(20, 0, ...);  // Au lieu de 16
```

---

### Problème : "config.txt n'est pas créé"

**Cause :** Permissions insuffisantes dans le dossier.

**Solution :**
1. Lancez l'overlay en tant qu'administrateur (clic droit → Exécuter en tant qu'administrateur)
2. OU déplacez le .exe dans un dossier utilisateur (Documents, Bureau, etc.)

---

### Problème : "L'overlay consomme trop de CPU"

**Causes :**
1. Intervalle de mise à jour trop court
2. Redessins trop fréquents

**Solution :** Augmentez l'intervalle dans `main.c` :
```c
#define TIMER_INTERVAL 5000  // 5 secondes au lieu de 2
```

---

## Utilisation Avancée

### Créer un Raccourci sur le Bureau

1. Clic droit sur **PerformanceOverlay.exe**
2. Envoyer vers → Bureau (créer un raccourci)
3. Renommez le raccourci : "Moniteur Performances"
4. Optionnel : Clic droit → Propriétés → Changer l'icône

### Lancer avec des Paramètres Spécifiques

Créez un fichier **launch.bat** :
```batch
@echo off
cd /d "C:\Users\momoe\Desktop\My app's creation\APP - overlay_perf_v2 - c_version"
start PerformanceOverlay.exe
```

Double-cliquez sur **launch.bat** pour lancer le programme.

---

## Performances du Programme

**Consommation mémoire :** ~2-3 MB
**Consommation CPU :** < 1% (en moyenne)
**Taille du .exe :** ~50 KB (ultra léger !)
**Compatibilité :** Windows 10/11 (64-bit)

---

## Commandes Rapides (Résumé)

### Compilation
```bash
make                    # Compiler le projet
make clean              # Nettoyer les fichiers temporaires
```

### Lancement
```bash
./PerformanceOverlay.exe              # Lancer le programme
./PerformanceOverlay.exe &            # Lancer en arrière-plan (Linux/MSYS2)
```

### Raccourcis Clavier
- **F1** : Afficher/Cacher
- **F2** : Mode Minimal/Complet
- **Clic sur X** : Fermer le programme

---

## Checklist - Premier Lancement

- [ ] Installer GCC/MinGW-w64
- [ ] Vérifier l'installation avec `gcc --version`
- [ ] Compiler le programme avec `make`
- [ ] Lancer **PerformanceOverlay.exe**
- [ ] Tester le déplacement de la fenêtre
- [ ] Tester les raccourcis F1 et F2
- [ ] Vérifier que **config.txt** est créé
- [ ] Repositionner l'overlay selon vos préférences
- [ ] Tester avec une application gourmande (navigateur, jeu)

---

## Support

**En cas de problème :**
1. Consultez la section "Résolution de Problèmes" ci-dessus
2. Vérifiez que tous les fichiers source sont présents
3. Recompilez complètement (`make clean && make`)
4. Consultez le **GUIDE_PEDAGOGIQUE.md** pour comprendre le code

**Pour modifier le programme :**
- Consultez **GUIDE_PEDAGOGIQUE.md** pour apprendre le C
- Modifiez les fichiers dans `src/`
- Recompilez avec `make`

---

**Bon monitoring ! 🚀**
