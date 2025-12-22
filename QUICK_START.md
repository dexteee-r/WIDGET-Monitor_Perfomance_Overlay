# Performance Overlay - Guide de demarrage rapide

## Installation

1. Extraire tous les fichiers du ZIP
2. Double-cliquer sur `PerformanceOverlay_v2.exe`
3. Le widget apparait automatiquement en haut a gauche de l'ecran

## Utilisation

### Raccourcis clavier

- **F2** : Basculer entre mode complet / minimal
- **F3** : Masquer / Afficher le widget
- **Clic droit** : Fermer le widget

### Deplacer le widget

1. Cliquer et maintenir le bouton gauche de la souris
2. Deplacer le widget ou vous voulez
3. Relacher le bouton

La position est sauvegardee automatiquement dans `config.txt`

## Metriques affichees

- **CPU** : Utilisation globale du processeur (%)
- **RAM** : Memoire utilisee / totale (GB)
- **Disk** : Espace disque utilise (%)
- **Uptime** : Temps depuis le demarrage du systeme
- **Processes** : Nombre de processus actifs
- **CPU Freq** : Frequence actuelle du processeur (GHz)

## Configuration

Au premier lancement, un fichier `config.txt` est cree automatiquement avec :
- Position du widget (X, Y)
- Mode d'affichage (0 = complet, 1 = minimal)

Vous pouvez editer ce fichier manuellement si besoin.

## Problemes courants

### Le widget n'apparait pas
- Verifiez qu'il ne s'execute pas deja (Gestionnaire des taches)
- Supprimez `config.txt` et relancez le programme

### Le widget est hors ecran
- Supprimez `config.txt`
- Le widget se recentrera automatiquement au prochain lancement

## Support

Pour plus d'informations, consultez le [README.md](README.md) complet ou visitez le depot GitHub.