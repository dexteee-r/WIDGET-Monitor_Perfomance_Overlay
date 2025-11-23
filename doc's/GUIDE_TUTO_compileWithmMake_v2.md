🛠️ Solution : Installer MinGW-w64 avec Make
Option 1 : Installation Rapide avec Chocolatey (Recommandé)
Chocolatey est un gestionnaire de paquets pour Windows (comme apt sur Linux). Étape 1 : Installer Chocolatey Dans PowerShell en tant qu'Administrateur (clic droit → Exécuter en tant qu'administrateur), tapez :
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebRequest('https://community.chocolatey.org/install.ps1')).Content)
Étape 2 : Installer MinGW et Make Toujours dans PowerShell Admin :
choco install mingw -y
choco install make -y
Étape 3 : Fermer et rouvrir PowerShell Fermez PowerShell et ouvrez-le à nouveau (pas besoin d'admin cette fois). Étape 4 : Vérifier l'installation
gcc --version
make --version
Option 2 : Installation Manuelle de MSYS2 (Plus de contrôle)
Étape 1 : Télécharger MSYS2 Allez sur https://www.msys2.org/ et téléchargez l'installateur. Étape 2 : Installer MSYS2
Lancez l'installateur
Installez dans C:\msys64 (par défaut)
Une fois l'installation terminée, décochez "Run MSYS2 now" et cliquez Finish
Étape 3 : Ajouter au PATH Windows Dans PowerShell Admin, tapez :
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\msys64\ucrt64\bin", [EnvironmentVariableTarget]::Machine)
Étape 4 : Installer GCC et Make Ouvrez le terminal MSYS2 UCRT64 (depuis le menu Démarrer) et tapez :
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
Étape 5 : Créer un alias pour make Dans MSYS2, make s'appelle mingw32-make.exe. Pour l'utiliser comme make dans PowerShell, créez un alias : Dans PowerShell Admin :
Copy-Item "C:\msys64\ucrt64\bin\mingw32-make.exe" "C:\msys64\ucrt64\bin\make.exe"
Étape 6 : Fermer et rouvrir PowerShell
Option 3 : Compilation Sans Make (Solution Immédiate)
Si vous voulez compiler tout de suite sans installer Make, voici comment :
Write c:\Users\momoe\Desktop\My app's creation\APP - overlay_perf_v2 - c_version\compile.bat
⎿
61 lines
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
Write c:\Users\momoe\Desktop\My app's creation\APP - overlay_perf_v2 - c_version\compile.ps1
⎿
60 lines
# Script PowerShell de compilation pour Performance Overlay
# Utilise GCC (MinGW-w64) pour compiler le projet

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Compilation de Performance Overlay" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Créer le dossier build s'il n'existe pas
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

# Vérifier que GCC est installé
try {
    $null = & gcc --version 2>&1
} catch {
    Write-Host "ERREUR: GCC n'est pas installé ou n'est pas dans le PATH" -ForegroundColor Red
    Write-Host "Installez MinGW-w64 avec: choco install mingw" -ForegroundColor Yellow
    exit 1
}

# Compilation
$steps = @(
    @{name="main.c"; src="src/main.c"; obj="build/main.o"},
    @{name="performance.c"; src="src/performance.c"; obj="build/performance.o"},
    @{name="config.c"; src="src/config.c"; obj="build/config.o"},
    @{name="startup.c"; src="src/startup.c"; obj="build/startup.o"}
)

$i = 1
foreach ($step in $steps) {
    Write-Host "[$i/$($steps.Count)] Compilation de $($step.name)..." -ForegroundColor Yellow

    & gcc -Wall -O2 -c $step.src -o $step.obj -Iinclude 2>&1 | Out-String | Write-Host

    if ($LASTEXITCODE -ne 0) {
        Write-Host "ERREUR: Echec de la compilation de $($step.name)" -ForegroundColor Red
        exit 1
    }
    $i++
}

# Linkage
Write-Host "[$i/$($steps.Count + 1)] Linkage final..." -ForegroundColor Yellow
& gcc -Wall -O2 -mwindows build/main.o build/performance.o build/config.o build/startup.o -o PerformanceOverlay.exe -lgdi32 -luser32 -ladvapi32 2>&1 | Out-String | Write-Host

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERREUR: Echec du linkage" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host " Compilation reussie!" -ForegroundColor Green
Write-Host " Executable: PerformanceOverlay.exe" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Tapez './PerformanceOverlay.exe' pour lancer le programme" -ForegroundColor Cyan
Parfait ! J'ai créé deux scripts de compilation. Maintenant, choisissez votre méthode :
🎯 Trois Options pour Vous
Option A : Installation Rapide avec Chocolatey (5 minutes)
Ouvrez PowerShell en tant qu'Administrateur
Installez Chocolatey :
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
Installez MinGW et Make :
choco install mingw make -y
Fermez et rouvrez PowerShell (normal, pas admin)
Testez :
gcc --version
make --version
Compilez votre projet :
make
Option B : Utiliser le Script PowerShell (SANS Make)
Si vous avez seulement GCC installé (sans Make), utilisez mon script :
.\compile.ps1
Ce script fera exactement la même chose que make !
Option C : Utiliser le Script Batch (SANS Make)
Double-cliquez sur compile.bat ou tapez :
.\compile.bat
🔧 Quelle méthode préférez-vous ?
Recommandation : Si vous voulez utiliser make dans PowerShell à l'avenir, choisissez Option A (Chocolatey). C'est la plus propre. Sinon, utilisez Option B (le script PowerShell) pour compiler immédiatement.
⚡ Test Rapide : Avez-vous déjà GCC ?
Tapez dans PowerShell :
gcc --version
Si vous voyez une version, vous avez déjà GCC ! Dans ce cas, utilisez simplement :
Bash
⎿
Vérifier si GCC est dans le PATH
IN
where.exe gcc 2>$null || echo "GCC non trouve"
OUT
GCC non trouve
/usr/bin/bash: line 1: $null: ambiguous redirect
Vous n'avez pas encore GCC installé. Donc voici ce que je vous recommande :
📌 Ma Recommandation : Installez Chocolatey + MinGW
C'est la solution la plus simple et la plus propre. Suivez ces étapes :
1. Ouvrir PowerShell en Administrateur
Tapez "PowerShell" dans le menu Démarrer
Clic droit sur "Windows PowerShell"
Choisissez "Exécuter en tant qu'administrateur"
2. Installer Chocolatey
Copiez-collez cette commande complète :
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
Attendez que ça se termine (30 secondes environ).
3. Installer MinGW et Make
Toujours dans PowerShell Admin :
choco install mingw make -y
Cela prendra 2-3 minutes.
4. Fermer et rouvrir PowerShell
Fermez PowerShell Admin et ouvrez un PowerShell normal dans votre dossier projet.
5. Vérifier l'installation
gcc --version
make --version
Si vous voyez les versions, c'est bon ! 🎉
6. Compiler votre projet
make
🚀 Voulez-vous que je vous guide étape par étape ?
Dites-moi quand vous avez :
Ouvert PowerShell Admin
Tapé la commande d'installation de Chocolatey
Installé MinGW et Make
Et je vous aiderai pour la suite (y compris corriger le bug CPU avant de compiler) !