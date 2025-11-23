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
