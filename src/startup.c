/*
 * startup.c
 * Implémentation du démarrage automatique
 */

#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include "../include/startup.h"

/*
 * AddToStartup
 * Ajoute l'application au démarrage de Windows via le registre
 * Retourne TRUE si succès, FALSE sinon
 */
BOOL AddToStartup() {
    HKEY hKey;
    char exePath[MAX_PATH];
    LONG result;

    // Obtenir le chemin complet de l'exécutable
    GetModuleFileName(NULL, exePath, MAX_PATH);

    // Ouvrir la clé de registre pour le démarrage
    result = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,
        &hKey
    );

    if (result == ERROR_SUCCESS) {
        // Ajouter notre programme au démarrage
        result = RegSetValueEx(
            hKey,
            "PerformanceOverlay",      // Nom de l'entrée
            0,
            REG_SZ,                     // Type: chaîne de caractères
            (BYTE*)exePath,             // Chemin de l'exe
            strlen(exePath) + 1         // Taille avec '\0'
        );

        RegCloseKey(hKey);
        return (result == ERROR_SUCCESS);
    }

    return FALSE;
}
