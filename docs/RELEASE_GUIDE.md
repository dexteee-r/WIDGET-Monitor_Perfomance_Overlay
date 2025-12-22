# 📦 Guide de Release GitHub

## Étapes pour créer une Release v2.0

### 1️⃣ **Nettoyer le Projet**

```powershell
# Supprimer les fichiers de test
.\cleanup_for_release.bat
```

### 2️⃣ **Builder la Release**

```powershell
# Compiler et créer le package
.\build_release.bat
```

Cela créera un dossier `Performance-Overlay-v2.0/` avec tous les fichiers nécessaires.

### 3️⃣ **Créer le ZIP**

**Option A - Explorateur Windows** :
- Clic droit sur `Performance-Overlay-v2.0`
- "Envoyer vers" → "Dossier compressé"

**Option B - PowerShell** :
```powershell
Compress-Archive -Path "Performance-Overlay-v2.0" -DestinationPath "Performance-Overlay-v2.0.zip"
```

### 4️⃣ **Commit et Push**

```powershell
# Ajouter les changements
git add .

# Commit
git commit -m "🎉 Release v2.0 - Widget hors écran fix + Protection multi-écrans

- Fix: Widget invisible après changement de disposition d'écrans
- Ajout: Protection anti-hors-écran automatique
- Ajout: Centrage automatique si position invalide
- Amélioration: Validation des coordonnées au démarrage

Closes #XX"

# Push
git push origin main
```

### 5️⃣ **Créer la Release sur GitHub**

1. Aller sur https://github.com/VOTRE-USERNAME/VOTRE-REPO/releases/new

2. **Tag version** : `v2.0.0`

3. **Release title** : `🚀 Performance Overlay v2.0 - Multi-Screen Fix`

4. **Description** : (Copier depuis ci-dessous)

```markdown
# 🎉 Performance Overlay v2.0

## 🆕 Nouveautés Majeures

### 🛡️ Protection Multi-Écrans
- ✅ **Fix du widget invisible** : Plus de problème après changement de disposition d'écrans
- ✅ **Validation automatique** : Détection et correction des positions invalides
- ✅ **Centrage intelligent** : Repositionnement automatique si hors écran

### ⌨️ Raccourcis Améliorés
- 🔄 **F3** remplace F1 (évite le conflit avec l'aide Windows 11)
- ✅ **F2** : Mode minimal/complet

### 📊 Métriques Complètes
- 🖥️ CPU avec fréquence (GHz)
- 💾 RAM détaillée (GB)
- 💿 Disque C:
- ⏱️ Uptime système
- 📊 Nombre de processus

## 📥 Installation

1. Téléchargez `Performance-Overlay-v2.0.zip`
2. Extrayez tous les fichiers
3. Lancez `PerformanceOverlay_v2.exe`
4. Le widget apparaît au centre de votre écran !

## 🐛 Corrections

- 🔧 Widget invisible après changement de config multi-écrans
- 🎯 Position du bouton X
- 🖥️ Monitoring CPU (utilise maintenant GetSystemTimes)

## 📝 Notes Complètes

Consultez [CHANGELOG.md](https://github.com/VOTRE-USERNAME/VOTRE-REPO/blob/main/CHANGELOG.md) pour la liste complète.

## 💡 Support

Si l'icône ne s'affiche pas : Lancez `refresh_icon.bat`

---

**Compatibilité** : Windows 10/11 (64-bit)
**Taille** : ~200 KB
**Dépendances** : Aucune
```

5. **Attacher le fichier** : Glissez-déposez `Performance-Overlay-v2.0.zip`

6. Cochez **Set as the latest release**

7. Cliquez **Publish release** 🚀

### 6️⃣ **Vérifications Post-Release**

- [ ] Le ZIP se télécharge correctement
- [ ] L'exécutable fonctionne sur une machine propre
- [ ] Le README s'affiche bien sur GitHub
- [ ] Les badges sont à jour

---

## 🎯 Checklist Complète

Avant de publier la release :

- [x] Code nettoyé (pas de fichiers de test)
- [x] Compilation réussie sans warnings
- [x] CHANGELOG mis à jour
- [x] README à jour
- [x] Version incrémentée partout
- [ ] Screenshots à jour dans le repo
- [ ] Tests sur machine propre
- [ ] ZIP créé et testé
- [ ] Release notes rédigées
- [ ] Tag Git créé

---

## 📸 Assets à Ajouter (optionnel)

Pour rendre la release plus attractive :

1. **Captures d'écran** dans `Screenchots/`
2. **GIF animé** du widget en action
3. **Bannière** pour le README

---

**Bonne release ! 🚀**
