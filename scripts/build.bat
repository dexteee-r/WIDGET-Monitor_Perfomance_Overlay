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
    mkdir Performance-Overlay-v2.0 2>nul
    copy PerformanceOverlay_v2.exe Performance-Overlay-v2.0\ >nul
    copy README.md Performance-Overlay-v2.0\ >nul
    copy LICENSE Performance-Overlay-v2.0\ >nul
    copy CHANGELOG.md Performance-Overlay-v2.0\ >nul
    copy resources\icon.ico Performance-Overlay-v2.0\ >nul
    echo.
    echo Release creee dans Performance-Overlay-v2.0\
    echo Pour creer le ZIP : Compress-Archive -Path "Performance-Overlay-v2.0" -DestinationPath "Performance-Overlay-v2.0.zip"
    pause
    exit /b 0
)

echo Build normal...
make
if %errorlevel% neq 0 pause
