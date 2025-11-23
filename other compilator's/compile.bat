@echo off
REM Script de compilation pour Performance Overlay
REM Utilise GCC (MinGW-w64) pour compiler le projet

echo ========================================
echo  Compilation de Performance Overlay
echo ========================================
echo.

REM Créer le dossier build s'il n'existe pas
if not exist "build" mkdir build

echo [1/5] Compilation de main.c...
gcc -Wall -O2 -c src/main.c -o build/main.o -Iinclude
if %errorlevel% neq 0 (
    echo ERREUR: Echec de la compilation de main.c
    pause
    exit /b 1
)

echo [2/5] Compilation de performance.c...
gcc -Wall -O2 -c src/performance.c -o build/performance.o -Iinclude
if %errorlevel% neq 0 (
    echo ERREUR: Echec de la compilation de performance.c
    pause
    exit /b 1
)

echo [3/5] Compilation de config.c...
gcc -Wall -O2 -c src/config.c -o build/config.o -Iinclude
if %errorlevel% neq 0 (
    echo ERREUR: Echec de la compilation de config.c
    pause
    exit /b 1
)

echo [4/5] Compilation de startup.c...
gcc -Wall -O2 -c src/startup.c -o build/startup.o -Iinclude
if %errorlevel% neq 0 (
    echo ERREUR: Echec de la compilation de startup.c
    pause
    exit /b 1
)

echo [5/5] Linkage final...
gcc -Wall -O2 -mwindows build/main.o build/performance.o build/config.o build/startup.o -o PerformanceOverlay.exe -lgdi32 -luser32 -ladvapi32
if %errorlevel% neq 0 (
    echo ERREUR: Echec du linkage
    pause
    exit /b 1
)

echo.
echo ========================================
echo  Compilation reussie!
echo  Executable: PerformanceOverlay.exe
echo ========================================
echo.
echo Tapez 'PerformanceOverlay.exe' pour lancer le programme
pause
