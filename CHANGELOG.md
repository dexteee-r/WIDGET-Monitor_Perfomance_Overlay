# Changelog

Toutes les modifications notables de ce projet seront documentées dans ce fichier.

Le format est basé sur [Keep a Changelog](https://keepachangelog.com/fr/1.0.0/),
et ce projet adhère au [Semantic Versioning](https://semver.org/lang/fr/).

## [2.0.0] - 2025-01-23

### Ajouté
- 🎨 **Nouveau design moderne** avec fond gris foncé et bordure bleue Windows 11
- 📊 **Barres de progression visuelles** avec code couleur (vert/orange/rouge)
- 💾 **RAM en GB** : Affichage détaillé (ex: 8.5 / 16.0 GB)
- ⏱️ **Uptime système** : Temps depuis le démarrage (heures + minutes)
- 📊 **Nombre de processus** actifs
- 🖥️ **Fréquence CPU** en GHz
- 🎯 **Icône personnalisée** pour l'exécutable
- 📝 **Métadonnées** dans les propriétés du fichier
- 🔧 **Script de nettoyage** du cache d'icônes Windows (`refresh_icon.bat`)

### Modifié
- ⌨️ **Raccourci F1 → F3** pour éviter le conflit avec l'aide Windows 11
- 📏 **Dimensions de la fenêtre** : 280x240px (mode complet), 280x120px (mode minimal)
- 🎨 **Police** : Segoe UI avec ClearType
- 🔄 **Monitoring CPU** : Correction du bug (utilise maintenant `GetSystemTimes()` au lieu de `GetProcessTimes()`)

### Corrigé
- 🐛 **Bug CPU** : Affichait l'utilisation du processus au lieu du système
- 🎯 **Position du bouton X** : Repositionné en haut à droite
- 🖼️ **Cache d'icônes** : Ajout d'un script de rafraîchissement

## [1.0.0] - 2024-XX-XX

### Ajouté
- ✨ **Première version** du Performance Overlay
- 🖥️ Monitoring CPU, RAM, Disque
- 🪟 Fenêtre transparente toujours au premier plan
- 🔑 Raccourcis clavier (F1, F2)
- 💾 Sauvegarde de configuration
- 🚀 Démarrage automatique Windows
- 📖 Guide pédagogique complet (714 lignes)

---
### A Ajouté : 
affiché le run time de la machine (depuis combien de temps la machine est allumé)



## Types de modifications

- **Ajouté** : Nouvelles fonctionnalités
- **Modifié** : Changements dans les fonctionnalités existantes
- **Déprécié** : Fonctionnalités bientôt supprimées
- **Supprimé** : Fonctionnalités supprimées
- **Corrigé** : Corrections de bugs
- **Sécurité** : Corrections de vulnérabilités
