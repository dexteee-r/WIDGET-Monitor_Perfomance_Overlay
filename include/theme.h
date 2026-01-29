/*
 * theme.h
 * Systeme de themes/skins pour l'interface
 */

#ifndef THEME_H
#define THEME_H

#include <windows.h>

// Structure pour un theme
typedef struct {
    const char* name;
    COLORREF bg;
    COLORREF panel;
    COLORREF border;
    COLORREF accent;
    COLORREF accent2;
    COLORREF text;
    COLORREF textMuted;
} ThemeSkin;

// Couleurs actives du theme (accessibles globalement)
extern COLORREF g_colorBg;
extern COLORREF g_colorPanel;
extern COLORREF g_colorBorder;
extern COLORREF g_colorAccent;
extern COLORREF g_colorAccent2;
extern COLORREF g_colorText;
extern COLORREF g_colorTextMuted;

// Acces aux themes
extern ThemeSkin g_themes[];
extern int g_themeCount;
extern int g_selectedTheme;

// Fonctions
void ApplyTheme(int themeIndex);
int GetThemeCount(void);
const char* GetThemeName(int index);
ThemeSkin* GetTheme(int index);

#endif // THEME_H
