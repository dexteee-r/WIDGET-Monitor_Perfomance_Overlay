@echo off
REM Script pour rafraîchir le cache d'icônes Windows

echo ========================================
echo  Rafraichissement du cache d'icones
echo ========================================
echo.

REM Arrêter l'Explorateur Windows
echo [1/4] Arret de l'Explorateur Windows...
taskkill /f /im explorer.exe

REM Supprimer le cache d'icônes
echo [2/4] Suppression du cache d'icones...
cd /d %userprofile%\AppData\Local\Microsoft\Windows\Explorer
attrib -h iconcache_*.db
del iconcache_*.db /a
del thumbcache_*.db /a

REM Supprimer aussi le cache dans AppData\Local
cd /d %localappdata%
if exist IconCache.db del IconCache.db /a

REM Redémarrer l'Explorateur
echo [3/4] Redemarrage de l'Explorateur...
start explorer.exe

echo [4/4] Termine!
echo.
echo Le cache d'icones a ete vide.
echo Recompilez maintenant votre programme avec: make clean && make
echo.
pause
