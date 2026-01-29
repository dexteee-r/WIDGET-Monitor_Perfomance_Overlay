/*
 * tray.c
 * Implementation de la gestion du system tray
 */

#include <windows.h>
#include <shellapi.h>
#include <string.h>
#include "../include/tray.h"

// Variables globales du module
static NOTIFYICONDATA g_nid = {0};
static BOOL g_trayIconAdded = FALSE;

/*
 * CreateTrayIcon
 * Cree l'icone dans le system tray
 */
void CreateTrayIcon(HWND hwnd, HINSTANCE hInstance) {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    if (!g_nid.hIcon) {
        g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    strcpy(g_nid.szTip, "Performance Overlay v3");

    Shell_NotifyIcon(NIM_ADD, &g_nid);
    g_trayIconAdded = TRUE;
}

/*
 * RemoveTrayIcon
 * Supprime l'icone du system tray
 */
void RemoveTrayIcon(void) {
    if (g_trayIconAdded) {
        Shell_NotifyIcon(NIM_DELETE, &g_nid);
        g_trayIconAdded = FALSE;
    }
}

/*
 * ShowTrayMenu
 * Affiche le menu contextuel du system tray
 */
void ShowTrayMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();

    // Options du menu
    if (IsWindowVisible(hwnd)) {
        AppendMenu(hMenu, MF_STRING, ID_TRAY_SHOW, "Masquer");
    } else {
        AppendMenu(hMenu, MF_STRING, ID_TRAY_SHOW, "Afficher");
    }
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, ID_TRAY_SETTINGS, "Parametres...");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Quitter");

    // Necessaire pour que le menu se ferme correctement
    SetForegroundWindow(hwnd);

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

    DestroyMenu(hMenu);
}
