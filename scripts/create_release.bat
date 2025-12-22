@echo off
REM Script automatise de creation de release GitHub
REM Cree le package binaire et le ZIP pret pour upload

echo.
echo ========================================
echo  CREATION DE RELEASE GITHUB
echo ========================================
echo.

REM === VERIFICATION PREREQUIS ===
echo [1/6] Verification des prerequis...

REM Verifier que nous sommes dans le bon dossier
if not exist "Makefile" (
    echo ERREUR: Makefile introuvable. Lancez ce script depuis la racine du projet.
    pause
    exit /b 1
)

if not exist "src" (
    echo ERREUR: Dossier src/ introuvable.
    pause
    exit /b 1
)

echo   - Dossiers OK

REM === VERIFIER GIT STATUS ===
echo.
echo [2/6] Verification de l'etat Git...
git status --short
if %errorlevel% neq 0 (
    echo AVERTISSEMENT: Git non initialise ou erreur
)
echo.
echo Assurez-vous que tout est commit et push avant de continuer !
echo.
pause

REM === COMPILATION ===
echo.
echo [3/6] Compilation de la version release...
make clean
make
if %errorlevel% neq 0 (
    echo.
    echo ERREUR: La compilation a echoue !
    echo Verifiez les erreurs ci-dessus.
    pause
    exit /b 1
)

REM === CREATION DU PACKAGE BINAIRE ===
echo.
echo [4/6] Creation du package binaire...

REM Nettoyer l'ancien package
if exist Performance-Overlay-v2.0 rd /s /q Performance-Overlay-v2.0
mkdir Performance-Overlay-v2.0

REM Copier les fichiers essentiels
echo   - Copie de l'executable...
copy PerformanceOverlay_v2.exe Performance-Overlay-v2.0\ >nul

echo   - Copie de la documentation...
copy README.md Performance-Overlay-v2.0\ >nul
copy LICENSE Performance-Overlay-v2.0\ >nul
copy CHANGELOG.md Performance-Overlay-v2.0\ >nul
copy QUICK_START.md Performance-Overlay-v2.0\ >nul

echo   - Copie des ressources...
copy resources\icon.ico Performance-Overlay-v2.0\ >nul

echo   - Creation du fichier d'exemple de configuration...
(
echo # Configuration par defaut - Performance Overlay
echo # Ce fichier sera cree automatiquement au premier lancement
echo #
echo # Format : X Y Mode
echo # X, Y : Position du widget en pixels
echo # Mode : 0 = complet, 1 = minimal
echo #
echo # Exemple :
echo # 100 100 0
) > Performance-Overlay-v2.0\config.example.txt

REM === CREATION DU ZIP ===
echo.
echo [5/6] Creation du fichier ZIP...

REM Supprimer l'ancien ZIP si existe
if exist Performance-Overlay-v2.0.zip del /q Performance-Overlay-v2.0.zip

REM Creer le ZIP avec PowerShell
powershell -Command "Compress-Archive -Path 'Performance-Overlay-v2.0' -DestinationPath 'Performance-Overlay-v2.0.zip' -Force"

if %errorlevel% neq 0 (
    echo.
    echo ERREUR: Impossible de creer le ZIP
    pause
    exit /b 1
)

REM === VERIFICATION ===
echo.
echo [6/6] Verification du package...

echo.
echo Contenu du package binaire :
dir /b Performance-Overlay-v2.0

echo.
echo Taille du ZIP :
dir Performance-Overlay-v2.0.zip | find "Performance-Overlay-v2.0.zip"

REM === RESUME ===
echo.
echo ========================================
echo  RELEASE CREEE AVEC SUCCES !
echo ========================================
echo.
echo Fichier cree : Performance-Overlay-v2.0.zip
echo.
echo PROCHAINES ETAPES :
echo.
echo 1. Verifiez que tout est commit et push sur GitHub :
echo    git status
echo    git push origin main
echo.
echo 2. Allez sur GitHub pour creer la release :
echo    https://github.com/VOTRE-USERNAME/Widget-perf_overlay/releases/new
echo.
echo 3. Remplissez le formulaire :
echo    - Tag version : v2.X
echo    - Release title : Performance Overlay v2.X
echo    - Description : Consultez RELEASE_WORKFLOW.md pour le template
echo    - Uploadez : Performance-Overlay-v2.0.zip
echo    - Cochez "Set as the latest release"
echo.
echo 4. Cliquez sur "Publish release"
echo.
echo Pour plus de details, consultez RELEASE_WORKFLOW.md
echo.
pause
