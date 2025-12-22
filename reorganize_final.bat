@echo off
REM REORGANISATION FINALE - Structure GitHub professionnelle
REM Inclut simplification de la documentation

echo ========================================
echo  REORGANISATION FINALE DU PROJET
echo ========================================
echo.

REM === ETAPE 1 : CREER LA STRUCTURE ===
echo [1/9] Creation de la structure...
if not exist "docs" mkdir docs
if not exist "docs\dev" mkdir docs\dev
if not exist "scripts" mkdir scripts
if not exist "screenshots" mkdir screenshots
if not exist "resources" mkdir resources

REM === ETAPE 2 : FUSIONNER doc's ET docs/ ===
echo [2/9] Fusion de doc's et docs/...
if exist "doc's" (
    echo   - Deplacement du contenu de doc's vers docs/...
    xcopy /E /I /Y "doc's\*" docs\ 2>nul
    rd /S /Q "doc's" 2>nul
)

REM === ETAPE 3 : CONSOLIDER LA DOCUMENTATION ===
echo [3/9] Consolidation intelligente de la documentation...

REM Garder à la racine : README.md, CHANGELOG.md, LICENSE
REM Ces fichiers sont essentiels pour GitHub et doivent rester visibles

REM Documentation technique -> docs/dev/
move /Y ARCHITECTURE.md docs\dev\ 2>nul
move /Y PROJECT_STRUCTURE.md docs\dev\ 2>nul
move /Y REFACTORING_GUIDE.md docs\dev\ 2>nul
move /Y SUMMARY_REFACTORING.md docs\dev\ 2>nul
move /Y CHANGELOG_V3.md docs\dev\ 2>nul

REM Guides pratiques -> docs/
move /Y RELEASE_GUIDE.md docs\dev\ 2>nul
move /Y QUICK_RELEASE.md docs\dev\ 2>nul
move /Y REORGANISATION_FINALE.md docs\dev\ 2>nul

REM Déplacer les guides existants dans docs/ vers docs/ (au cas où)
if exist "docs\GUIDE_PEDAGOGIQUE.md" (
    echo   - Guides deja dans docs/
)

REM === ETAPE 4 : CREER UN INDEX CLAIR DANS docs/ ===
echo [4/9] Creation de l'index de documentation...
(
echo # Documentation - Performance Overlay
echo.
echo ## Pour les utilisateurs
echo.
echo - [Guide d'utilisation](GUIDE_UTILISATION.md^) - Comment installer et utiliser le widget
echo - [Guide de compilation](GUIDE_TUTO_compileWithmMake_v2.md^) - Compiler depuis les sources
echo.
echo ## Pour les developpeurs
echo.
echo Consultez le dossier dev/ pour la documentation technique :
echo.
echo - [Architecture](dev/ARCHITECTURE.md^) - Architecture du code
echo - [Structure du projet](dev/PROJECT_STRUCTURE.md^) - Organisation des fichiers
echo - [Guide de refactoring](dev/REFACTORING_GUIDE.md^) - Historique des refactorings
echo - [Guide de release](dev/RELEASE_GUIDE.md^) - Comment creer une release
echo.
echo ## Guide pedagogique
echo.
echo Le [Guide pedagogique](GUIDE_PEDAGOGIQUE.md^) explique le code ligne par ligne pour apprendre.
echo.
) > docs\README.md

REM === ETAPE 5 : DEPLACER LES RESSOURCES ===
echo [5/9] Deplacement des ressources dans resources/...
move /Y icon.ico resources\ 2>nul
move /Y resources.rc resources\ 2>nul
if exist config.ini (
    copy /Y config.ini resources\config.ini.example 2>nul
)

REM Creer README dans resources/
(
echo # Resources Directory
echo.
echo Ce dossier contient toutes les ressources du projet :
echo.
echo - **icon.ico** : Icone de l'application
echo - **resources.rc** : Fichier de ressources Windows
echo - **config.ini.example** : Exemple de fichier de configuration
echo.
echo Le fichier config.ini sera genere automatiquement au premier lancement.
) > resources\README.md

REM === ETAPE 6 : CREER LE SCRIPT BUILD UNIFIE ===
echo [6/9] Creation du script de build unifie...
(
echo @echo off
echo REM Script de build unifie - Performance Overlay
echo REM Usage: build.bat [clean^|release]
echo.
echo if "%%1"=="clean" ^(
echo     echo Nettoyage...
echo     make clean
echo     exit /b 0
echo ^)
echo.
echo if "%%1"=="release" ^(
echo     echo ========================================
echo     echo  BUILD RELEASE v2.0
echo     echo ========================================
echo     make clean
echo     make
echo     if %%errorlevel%% neq 0 ^(
echo         echo ERREUR: Echec de la compilation
echo         pause
echo         exit /b 1
echo     ^)
echo     echo.
echo     echo Creation du package...
echo     mkdir Performance-Overlay-v2.0 2^>nul
echo     copy PerformanceOverlay_v2.exe Performance-Overlay-v2.0\ ^>nul
echo     copy README.md Performance-Overlay-v2.0\ ^>nul
echo     copy LICENSE Performance-Overlay-v2.0\ ^>nul
echo     copy CHANGELOG.md Performance-Overlay-v2.0\ ^>nul
echo     copy resources\icon.ico Performance-Overlay-v2.0\ ^>nul
echo     echo.
echo     echo Release creee dans Performance-Overlay-v2.0\
echo     echo Pour creer le ZIP : Compress-Archive -Path "Performance-Overlay-v2.0" -DestinationPath "Performance-Overlay-v2.0.zip"
echo     pause
echo     exit /b 0
echo ^)
echo.
echo echo Build normal...
echo make
echo if %%errorlevel%% neq 0 pause
) > scripts\build.bat

move /Y refresh_icon.bat scripts\ 2>nul
del /Q build.bat 2>nul
del /Q build_release.bat 2>nul
del /Q cleanup_for_release.bat 2>nul

REM === ETAPE 7 : DEPLACER LES SCREENSHOTS ===
echo [7/9] Organisation des screenshots...
if exist "Screenchots" (
    xcopy /E /I /Y "Screenchots\*" screenshots\ 2>nul
    rd /S /Q "Screenchots" 2>nul
)

REM === ETAPE 8 : SUPPRIMER LES FICHIERS INUTILES ===
echo [8/9] Suppression des fichiers inutiles...
del /Q create_icon.py 2>nul
del /Q PerformanceOverlay_v2_debug.exe 2>nul
del /Q test_*.* 2>nul
del /Q reorganize.bat 2>nul
del /Q reorganize_clean.bat 2>nul
del /Q reorganize_clean.bat.old 2>nul
rd /S /Q inspi 2>nul
rd /S /Q "other compilator's" 2>nul
rd /S /Q Performance-Overlay-v2.0 2>nul

REM === ETAPE 9 : NETTOYER BUILD ET CONFIG ===
echo [9/9] Nettoyage final...
rd /S /Q build 2>nul
del /Q config.txt 2>nul
del /Q config.ini 2>nul

echo.
echo ========================================
echo  REORGANISATION TERMINEE !
echo ========================================
echo.
echo Structure finale :
echo.
echo Racine (visible sur GitHub) :
echo   README.md          - Presentation du projet
echo   CHANGELOG.md       - Historique des versions
echo   LICENSE            - Licence du projet
echo   Makefile           - Compilation
echo   .gitignore         - Fichiers ignores
echo.
echo Dossiers :
echo   src/               - Code source
echo   include/           - Headers
echo   docs/              - Documentation utilisateur (3 guides + README)
echo   docs/dev/          - Documentation technique (developpeurs)
echo   resources/         - Icones, RC, configs
echo   screenshots/       - Captures d'ecran
echo   scripts/           - Scripts de build
echo.
echo Documentation simplifiee :
echo   docs/README.md              - INDEX (point d'entree)
echo   docs/GUIDE_UTILISATION.md   - Pour utilisateurs
echo   docs/GUIDE_PEDAGOGIQUE.md   - Pour apprendre
echo   docs/GUIDE_TUTO_compileWithmMake_v2.md - Compilation
echo   docs/dev/                   - Technique (architecture, etc.)
echo.
echo.
echo PROCHAINES ETAPES :
echo   1. Verifier la structure : tree /F
echo   2. Tester la compilation : make
echo   3. Builder release : scripts\build.bat release
echo   4. Commit et push sur GitHub
echo.
echo Appuyez sur une touche pour afficher l'arborescence...
pause >nul
tree /F /A
echo.
pause
