# ⚡ Quick Release Commands

## 🚀 Commandes Rapides pour Release v2.0

### **Étape 1 : Préparation**

```powershell
# Nettoyer les fichiers de test
.\cleanup_for_release.bat

# OU manuellement
Remove-Item test_window.c, test_window.exe, compile_test.bat, test_visible.bat, cleanup_for_release.bat -ErrorAction SilentlyContinue
```

### **Étape 2 : Build Release**

```powershell
# Builder et créer le package
.\build_release.bat

# OU manuellement
make clean
make
mkdir Performance-Overlay-v2.0
copy PerformanceOverlay_v2.exe, README.md, LICENSE, CHANGELOG.md, icon.ico, refresh_icon.bat Performance-Overlay-v2.0\
```

### **Étape 3 : Créer le ZIP**

```powershell
Compress-Archive -Path "Performance-Overlay-v2.0" -DestinationPath "Performance-Overlay-v2.0.zip" -Force
```

### **Étape 4 : Git**

```powershell
git status
git add .
git commit -m "🎉 Release v2.0 - Multi-Screen Fix & Auto-Centering"
git push origin main
```

### **Étape 5 : Tag**

```powershell
git tag -a v2.0.0 -m "Release v2.0.0 - Multi-Screen Protection"
git push origin v2.0.0
```

---

## 📋 One-Liner (Tout en Une Fois)

```powershell
.\cleanup_for_release.bat && .\build_release.bat && Compress-Archive -Path "Performance-Overlay-v2.0" -DestinationPath "Performance-Overlay-v2.0.zip" -Force && echo "✅ Release prête : Performance-Overlay-v2.0.zip"
```

---

## 🌐 Sur GitHub

1. Aller sur : `https://github.com/VOTRE-USERNAME/VOTRE-REPO/releases/new`
2. Tag : `v2.0.0`
3. Title : `🚀 Performance Overlay v2.0 - Multi-Screen Fix`
4. Upload : `Performance-Overlay-v2.0.zip`
5. Publish ! 🎉

---

## ✅ Vérification Post-Release

```powershell
# Tester le ZIP
Expand-Archive Performance-Overlay-v2.0.zip -DestinationPath test-extract
cd test-extract\Performance-Overlay-v2.0
.\PerformanceOverlay_v2.exe
```

Si ça fonctionne → **Release OK** ! 🚀
