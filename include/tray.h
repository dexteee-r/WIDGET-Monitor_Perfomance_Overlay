/*
 * tray.h
 * Gestion de l'icone dans le system tray
 */

#ifndef TRAY_H
#define TRAY_H

#include <windows.h>

// Message personnalise pour le tray
#define WM_TRAYICON (WM_USER + 1)

// IDs des commandes du menu tray
#define ID_TRAY_SHOW     1001
#define ID_TRAY_SETTINGS 1002
#define ID_TRAY_EXIT     1003

// Fonctions
void CreateTrayIcon(HWND hwnd, HINSTANCE hInstance);
void RemoveTrayIcon(void);
void ShowTrayMenu(HWND hwnd);

#endif // TRAY_H
