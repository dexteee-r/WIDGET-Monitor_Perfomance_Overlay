/*
 * ui_draw.h
 * Fonctions de dessin de l'interface utilisateur
 */

#ifndef UI_DRAW_H
#define UI_DRAW_H

#include <windows.h>

// Variables externes necessaires
extern int g_currentPage;
extern const char* g_pageNames[];

// Fonctions de dessin
void DrawTabs(HDC hdc, int width, HFONT hFontSmall);
void DrawSettingsPage(HDC hdc, int width, int height, HFONT hFontNormal, HFONT hFontSmall);
void DrawTaskKillerPage(HDC hdc, int width, int height, HFONT hFontNormal, HFONT hFontSmall);

#endif // UI_DRAW_H
