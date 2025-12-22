# Scripts du projet

Ce dossier contient les scripts utilitaires pour le projet Performance Overlay.

## Scripts disponibles

### build.bat
Script de compilation unifie.

**Usage :**
```batch
build.bat [clean|release]
```

**Exemples :**
```batch
# Compilation normale
build.bat

# Nettoyer les fichiers compiles
build.bat clean

# Creer un package de release
build.bat release
```

### create_release.bat
Script automatise pour creer une release GitHub complete.

**Usage :**
```batch
create_release.bat
```

**Ce qu'il fait :**
1. Verifie les prerequis
2. Compile l'executable
3. Cree le package binaire
4. Genere le fichier ZIP
5. Affiche les prochaines etapes

**Resultat :** Fichier `Performance-Overlay-v2.0.zip` pret pour upload sur GitHub

### refresh_icon.bat
Nettoie le cache d'icones Windows pour forcer le rafraichissement.

**Usage :**
```batch
refresh_icon.bat
```

Utilisez ce script si l'icone ne se met pas a jour apres modification.

## Workflow de release

Pour creer une nouvelle release GitHub :

1. **Methode automatique (recommandee) :**
   ```batch
   scripts\create_release.bat
   ```

2. **Methode manuelle :**
   ```batch
   scripts\build.bat release
   Compress-Archive -Path "Performance-Overlay-v2.0" -DestinationPath "Performance-Overlay-v2.0.zip" -Force
   ```

Consultez `RELEASE_WORKFLOW.md` (a la racine) pour le guide complet.
