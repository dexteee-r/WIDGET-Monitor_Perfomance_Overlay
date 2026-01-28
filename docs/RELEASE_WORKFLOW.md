# Guide : Creer une Release GitHub

Ce guide explique comment creer une release complete sur GitHub avec le package binaire pour les utilisateurs.

## Methode automatique (RECOMMANDE)

Utilisez le script automatise qui fait tout pour vous :

```powershell
.\scripts\create_release.bat
```

Le script va :
1. Nettoyer le projet
2. Compiler l'executable
3. Creer le package binaire
4. Generer le ZIP
5. Afficher le resume et les prochaines etapes

## Methode manuelle

### Etape 1 : Preparer le code

```powershell
# Verifier que tout est commit
git status

# Si des modifications existent, les committer
git add .
git commit -m "Release v2.X - Description des changements"
git push origin main
```

### Etape 2 : Compiler et creer le package

```powershell
# Compiler la version release
.\scripts\build.bat release

# Creer le ZIP
Compress-Archive -Path "Performance-Overlay-v2.0" -DestinationPath "Performance-Overlay-v2.0.zip" -Force
```

### Etape 3 : Verifier le contenu du ZIP

```powershell
# Verifier le contenu (optionnel)
Expand-Archive -Path "Performance-Overlay-v2.0.zip" -DestinationPath "temp_check" -Force
ls temp_check\Performance-Overlay-v2.0
rm -r temp_check
```

Le ZIP doit contenir uniquement :
- PerformanceOverlay_v2.exe
- README.md
- LICENSE
- CHANGELOG.md
- QUICK_START.md
- icon.ico
- config.example.txt

### Etape 4 : Creer la release sur GitHub

#### Via l'interface web (recommande si pas de GitHub CLI)

1. Allez sur : https://github.com/VOTRE-USERNAME/Widget-perf_overlay/releases
2. Cliquez sur **"Create a new release"**
3. Remplissez le formulaire :

**Tag version** : `v2.X` (exemple: v2.0, v2.1, etc.)

**Release title** : `Performance Overlay v2.X`

**Description** : Utilisez le template ci-dessous

**Fichier binaire** : Uploadez `Performance-Overlay-v2.0.zip`

4. Cochez **"Set as the latest release"**
5. Cliquez sur **"Publish release"**

#### Via GitHub CLI (si installe)

```powershell
gh release create v2.X Performance-Overlay-v2.0.zip `
  --title "Performance Overlay v2.X" `
  --notes-file RELEASE_NOTES.md
```

## Template de description de release

Copiez-collez ce template dans la description de votre release GitHub :

```markdown
## 🎉 Performance Overlay v2.X

### ✨ Nouveautes
- 🛡️ **Protection anti-hors-ecran** : Validation automatique des coordonnees au demarrage
- 📍 **Centrage automatique** : Le widget se recentre si la position est invalide
- ⌨️ **Nouveau raccourci F3** (au lieu de F1) pour eviter le conflit avec l'aide Windows 11
- 🎯 **Icone personnalisee** avec metadonnees dans les proprietes du fichier
- 🔧 **Script de nettoyage** du cache d'icones Windows

### 🐛 Corrections
- ✅ **Fix monitoring CPU** : Utilise maintenant `GetSystemTimes()` au lieu de `GetProcessTimes()`
- ✅ **Fix widget invisible** : Correction du probleme de position hors ecran apres changement de configuration multi-ecrans
- ✅ **Position du bouton X** repositionne en haut a droite

### 📊 Metriques affichees
- **CPU** : Utilisation globale du processeur (%)
- **RAM** : Memoire utilisee / totale (GB)
- **Disk** : Espace disque utilise sur C: (%)
- **Uptime** : Temps depuis le demarrage du systeme
- **Processes** : Nombre de processus actifs
- **CPU Freq** : Frequence actuelle du processeur (GHz)

### 📦 Installation
1. Telechargez `Performance-Overlay-v2.0.zip`
2. Decompressez le fichier
3. Lancez `PerformanceOverlay_v2.exe`
4. Consultez `QUICK_START.md` pour les details

### ⌨️ Raccourcis clavier
- **F2** : Basculer entre mode complet/minimal
- **F3** : Masquer/Afficher le widget
- **Clic droit** : Fermer le widget

### 🔧 Compilation depuis les sources
```bash
make
```

Voir [CHANGELOG.md](CHANGELOG.md) pour les details complets.
```

## Checklist avant publication

- [ ] Code compile sans erreurs (`make`)
- [ ] Tests manuels effectues (deplacement, raccourcis F2/F3)
- [ ] CHANGELOG.md mis a jour
- [ ] README.md a jour
- [ ] Tout est commit et push sur GitHub
- [ ] Package binaire cree avec `build.bat release`
- [ ] ZIP genere et verifie
- [ ] Tag version correct (v2.X)
- [ ] Description de release complete
- [ ] "Set as the latest release" coche

## Troubleshooting

### Le widget ne compile pas
```powershell
# Nettoyer et recompiler
make clean
make
```

### Le ZIP contient trop de fichiers
Le script `build.bat release` cree un package binaire propre avec uniquement les fichiers necessaires.
Ne zippez PAS le dossier racine du projet !

### Git refuse de push
```powershell
# Verifier les remotes
git remote -v

# Pull d'abord
git pull origin main

# Puis push
git push origin main
```

## Versionning

Utilisez le versionning semantique :
- **v2.0** : Version majeure (changements importants)
- **v2.1** : Version mineure (nouvelles fonctionnalites)
- **v2.0.1** : Patch (corrections de bugs)

## Apres la release

1. Verifiez que la release est visible sur GitHub
2. Testez le telechargement du ZIP
3. Verifiez que le ZIP se decompresse correctement
4. Testez l'executable sur une machine propre (si possible)
5. Annoncez la release (si applicable)

## Ressources

- Documentation GitHub : https://docs.github.com/en/repositories/releasing-projects-on-github
- Guide versionning semantique : https://semver.org/
