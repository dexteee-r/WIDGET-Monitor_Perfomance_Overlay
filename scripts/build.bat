@echo off
REM Script de build unifie - Performance Overlay
REM Usage: build.bat [clean|release]

if "%1"=="clean" (
    echo Nettoyage...
    make clean
    exit /b 0
)

if "%1"=="release" (
    echo ========================================
    echo  BUILD RELEASE v2.0
    echo ========================================
    make clean
    make
    if %errorlevel% neq 0 (
        echo ERREUR: Echec de la compilation
        pause
        exit /b 1
    )
    echo.
    echo Creation du package...

    REM Nettoyer l'ancien package si existe
    if exist Performance-Overlay-v2.0 rd /s /q Performance-Overlay-v2.0
    mkdir Performance-Overlay-v2.0

    REM Fichiers essentiels
    echo   - Copie de l'executable...
    copy PerformanceOverlay_v2.exe Performance-Overlay-v2.0\ >nul

    REM Documentation
    echo   - Copie de la documentation...
    copy README.md Performance-Overlay-v2.0\ >nul
    copy LICENSE Performance-Overlay-v2.0\ >nul
    copy CHANGELOG.md Performance-Overlay-v2.0\ >nul
    copy QUICK_START.md Performance-Overlay-v2.0\ >nul

    REM Ressources
    echo   - Copie des ressources...
    copy resources\icon.ico Performance-Overlay-v2.0\ >nul

    REM Exemple de configuration
    echo   - Creation du fichier d'exemple...
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

    echo.
    echo ========================================
    echo  PACKAGE CREE !
    echo ========================================
    echo.
    echo Contenu du package :
    dir /b Performance-Overlay-v2.0
    echo.
    echo Pour creer le ZIP :
    echo   Compress-Archive -Path "Performance-Overlay-v2.0" -DestinationPath "Performance-Overlay-v2.0.zip" -Force
    echo.
    pause
    exit /b 0
)

echo Build normal...
make
if %errorlevel% neq 0 pause
