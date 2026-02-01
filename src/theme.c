/*
 * theme.c
 * Implementation du systeme de themes/skins
 */

#include "../include/theme.h"

// Themes disponibles (index 5 = Custom, modifiable dynamiquement)
ThemeSkin g_themes[] = {
    {"Neon Dark",  RGB(8, 10, 18),   RGB(16, 20, 30),  RGB(40, 45, 60),   RGB(0, 230, 255),   RGB(255, 64, 180),  RGB(200, 210, 230), RGB(120, 130, 150)},
    {"Cyberpunk",  RGB(15, 5, 20),   RGB(30, 10, 40),  RGB(60, 30, 70),   RGB(255, 64, 180),  RGB(0, 255, 255),   RGB(255, 200, 255), RGB(150, 100, 150)},
    {"Matrix",     RGB(0, 10, 0),    RGB(0, 20, 5),    RGB(0, 40, 10),    RGB(0, 255, 65),    RGB(100, 255, 100), RGB(150, 255, 150), RGB(50, 150, 50)},
    {"Ocean",      RGB(5, 15, 25),   RGB(10, 30, 50),  RGB(20, 50, 80),   RGB(100, 200, 255), RGB(50, 150, 255),  RGB(200, 230, 255), RGB(100, 150, 180)},
    {"Sunset",     RGB(25, 10, 10),  RGB(40, 20, 15),  RGB(60, 35, 25),   RGB(255, 150, 50),  RGB(255, 100, 100), RGB(255, 220, 200), RGB(180, 130, 100)},
    {"Custom",     RGB(20, 10, 30),  RGB(30, 20, 45),  RGB(60, 40, 80),   RGB(200, 100, 255), RGB(255, 100, 150), RGB(230, 220, 255), RGB(140, 120, 160)},
};

int g_themeCount = 6;
int g_selectedTheme = 0;

// Couleurs actives du theme
COLORREF g_colorBg;
COLORREF g_colorPanel;
COLORREF g_colorBorder;
COLORREF g_colorAccent;
COLORREF g_colorAccent2;
COLORREF g_colorText;
COLORREF g_colorTextMuted;

/*
 * SetCustomTheme
 * Configure les couleurs du theme personnalise (index 5)
 */
void SetCustomTheme(COLORREF bg, COLORREF panel, COLORREF border,
                    COLORREF accent, COLORREF accent2,
                    COLORREF text, COLORREF textMuted) {
    g_themes[5].bg = bg;
    g_themes[5].panel = panel;
    g_themes[5].border = border;
    g_themes[5].accent = accent;
    g_themes[5].accent2 = accent2;
    g_themes[5].text = text;
    g_themes[5].textMuted = textMuted;
}

/*
 * ApplyTheme
 * Applique le theme selectionne aux couleurs globales
 */
void ApplyTheme(int themeIndex) {
    if (themeIndex < 0 || themeIndex >= g_themeCount) {
        themeIndex = 0;
    }
    g_selectedTheme = themeIndex;

    ThemeSkin* theme = &g_themes[themeIndex];
    g_colorBg = theme->bg;
    g_colorPanel = theme->panel;
    g_colorBorder = theme->border;
    g_colorAccent = theme->accent;
    g_colorAccent2 = theme->accent2;
    g_colorText = theme->text;
    g_colorTextMuted = theme->textMuted;
}

int GetThemeCount(void) {
    return g_themeCount;
}

const char* GetThemeName(int index) {
    if (index < 0 || index >= g_themeCount) return NULL;
    return g_themes[index].name;
}

ThemeSkin* GetTheme(int index) {
    if (index < 0 || index >= g_themeCount) return NULL;
    return &g_themes[index];
}
