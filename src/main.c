/*
 * main.c
 * Programme principal - Overlay de performance systeme
 *
 * Overlay temps reel minimaliste (style cyberdeck/TUI)
 */

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include "../include/performance.h"
#include "../include/config.h"
#include "../include/config_parser.h"
#include "../include/startup.h"
#include "../include/constants.h"
#include "../include/metric_plugin.h"
#include "../include/all_plugins.h"
#include "../include/taskkiller.h"

// Variables globales
HWND g_hwnd = NULL;              // Handle de la fenetre principale
ConfigINI g_config;              // Configuration de l'application (v3.0 - INI)
Config g_configLegacy;           // Configuration legacy (v2.0 - TXT) pour compatibilite
PerformanceData g_perfData;      // Donnees de performance (legacy, pour fallback)
BOOL g_isDragging = FALSE;       // Etat du deplacement de fenetre
POINT g_dragOffset;              // Offset lors du drag
char g_bannerTop[64] = "==[ SYSTEM OVERLAY v3 ]==";
char g_bannerBottom[64] = ":: PLUGIN SYSTEM ::";

// Systeme de pages
int g_currentPage = PAGE_PERF;   // Page actuelle (0=Perf, 1=Tasks, 2=Settings)
const char* g_pageNames[] = {"Perf", "Tasks", "Settings"};

// System Tray
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_SHOW 1001
#define ID_TRAY_SETTINGS 1002
#define ID_TRAY_EXIT 1003
static NOTIFYICONDATA g_nid = {0};
static BOOL g_trayIconAdded = FALSE;

static int GetWindowHeightForMode(BOOL minimal) {
    if (g_currentPage == PAGE_TASKS) {
        return WINDOW_HEIGHT_TASKS;
    }
    if (g_currentPage == PAGE_SETTINGS) {
        return WINDOW_HEIGHT_SETTINGS;
    }
    return minimal ? WINDOW_HEIGHT_MIN : WINDOW_HEIGHT_FULL;
}

// Structure pour les thèmes/skins
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

// Thèmes disponibles
static ThemeSkin g_themes[] = {
    {"Neon Dark",    RGB(8, 10, 18),    RGB(16, 20, 30),   RGB(40, 45, 60),    RGB(0, 230, 255),   RGB(255, 64, 180),  RGB(200, 210, 230), RGB(120, 130, 150)},
    {"Cyberpunk",    RGB(15, 5, 20),    RGB(30, 10, 40),   RGB(60, 30, 70),    RGB(255, 64, 180),  RGB(0, 255, 255),   RGB(255, 200, 255), RGB(150, 100, 150)},
    {"Matrix",       RGB(0, 10, 0),     RGB(0, 20, 5),     RGB(0, 40, 10),     RGB(0, 255, 65),    RGB(100, 255, 100), RGB(150, 255, 150), RGB(50, 150, 50)},
    {"Ocean",        RGB(5, 15, 25),    RGB(10, 30, 50),   RGB(20, 50, 80),    RGB(100, 200, 255), RGB(50, 150, 255),  RGB(200, 230, 255), RGB(100, 150, 180)},
    {"Sunset",       RGB(25, 10, 10),   RGB(40, 20, 15),   RGB(60, 35, 25),    RGB(255, 150, 50),  RGB(255, 100, 100), RGB(255, 220, 200), RGB(180, 130, 100)},
};
static int g_selectedTheme = 0;
static int g_themeCount = 5;

// Couleurs actives du thème (utilisées pour le dessin)
static COLORREF g_colorBg;
static COLORREF g_colorPanel;
static COLORREF g_colorBorder;
static COLORREF g_colorAccent;
static COLORREF g_colorAccent2;
static COLORREF g_colorText;
static COLORREF g_colorTextMuted;

/*
 * ApplyTheme
 * Applique le thème sélectionné aux couleurs globales
 */
static void ApplyTheme(int themeIndex) {
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

/*
 * DrawTabs
 * Dessine les onglets de navigation en haut du widget
 */
static void DrawTabs(HDC hdc, int width, HFONT hFontSmall) {
    int tabX = 16;
    int tabY = 12;

    SelectObject(hdc, hFontSmall);

    for (int i = 0; i < PAGE_COUNT; i++) {
        RECT tabRect = {tabX, tabY, tabX + TAB_WIDTH, tabY + TAB_HEIGHT};

        // Fond de l'onglet
        HBRUSH tabBrush;
        if (i == g_currentPage) {
            tabBrush = CreateSolidBrush(g_colorAccent2);
            SetTextColor(hdc, RGB(0, 0, 0));
        } else {
            tabBrush = CreateSolidBrush(g_colorBorder);
            SetTextColor(hdc, g_colorTextMuted);
        }
        FillRect(hdc, &tabRect, tabBrush);
        DeleteObject(tabBrush);

        // Texte de l'onglet
        SetBkMode(hdc, TRANSPARENT);
        DrawText(hdc, g_pageNames[i], -1, &tabRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        tabX += TAB_WIDTH + TAB_MARGIN;
    }
}

/*
 * DrawSettingsPage
 * Dessine la page des paramètres/skins
 */
static void DrawSettingsPage(HDC hdc, int width, int height, HFONT hFontNormal, HFONT hFontSmall) {
    int y = 50;  // Après les onglets

    SelectObject(hdc, hFontNormal);
    SetTextColor(hdc, g_colorAccent);
    TextOut(hdc, 16, y, ":: THEMES / SKINS ::", 20);
    y += 30;

    SelectObject(hdc, hFontSmall);

    // Afficher les thèmes disponibles
    for (int i = 0; i < g_themeCount; i++) {
        RECT themeRect = {16, y, width - 16, y + 28};

        // Fond du bouton thème
        HBRUSH themeBrush;
        if (i == g_selectedTheme) {
            themeBrush = CreateSolidBrush(g_themes[i].accent);
            SetTextColor(hdc, RGB(0, 0, 0));
        } else {
            themeBrush = CreateSolidBrush(g_themes[i].panel);
            SetTextColor(hdc, g_themes[i].text);
        }
        FillRect(hdc, &themeRect, themeBrush);
        DeleteObject(themeBrush);

        // Bordure
        HPEN borderPen = CreatePen(PS_SOLID, 1, g_themes[i].accent);
        SelectObject(hdc, borderPen);
        MoveToEx(hdc, themeRect.left, themeRect.top, NULL);
        LineTo(hdc, themeRect.right, themeRect.top);
        LineTo(hdc, themeRect.right, themeRect.bottom);
        LineTo(hdc, themeRect.left, themeRect.bottom);
        LineTo(hdc, themeRect.left, themeRect.top);
        DeleteObject(borderPen);

        // Aperçu couleur
        RECT previewRect = {20, y + 4, 40, y + 24};
        HBRUSH previewBrush = CreateSolidBrush(g_themes[i].bg);
        FillRect(hdc, &previewRect, previewBrush);
        DeleteObject(previewBrush);

        // Nom du thème
        SetBkMode(hdc, TRANSPARENT);
        RECT textRect = {50, y, width - 20, y + 28};
        DrawText(hdc, g_themes[i].name, -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Indicateur sélectionné
        if (i == g_selectedTheme) {
            SetTextColor(hdc, RGB(0, 0, 0));
            TextOut(hdc, width - 40, y + 6, "[OK]", 4);
        }

        y += 32;
    }

    // Instructions
    y += 10;
    SetTextColor(hdc, g_colorTextMuted);
    TextOut(hdc, 16, y, "Cliquez sur un theme pour l'appliquer", 37);
}

/*
 * DrawTaskKillerPage
 * Dessine le contenu de la page Task Killer
 */
static void DrawTaskKillerPage(HDC hdc, int width, int height, HFONT hFontNormal, HFONT hFontSmall) {
    TaskKillerData* data = GetTaskKillerData();
    int y = 50;  // Apres les onglets

    SelectObject(hdc, hFontSmall);

    // ===== TOGGLE ALL/PORTS =====
    const char* modeAll = "All";
    const char* modePorts = "Ports";
    int toggleX = 16;

    // Bouton "All"
    RECT allRect = {toggleX, y, toggleX + 40, y + 18};
    HBRUSH allBrush = CreateSolidBrush(data->view_mode == VIEW_MODE_ALL ? COLOR_MAGENTA : COLOR_BORDER);
    FillRect(hdc, &allRect, allBrush);
    DeleteObject(allBrush);
    SetTextColor(hdc, data->view_mode == VIEW_MODE_ALL ? RGB(0, 0, 0) : COLOR_TEXT_MUTED);
    DrawText(hdc, modeAll, -1, &allRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Bouton "Ports"
    RECT portsRect = {toggleX + 42, y, toggleX + 92, y + 18};
    HBRUSH portsBrush = CreateSolidBrush(data->view_mode == VIEW_MODE_PORTS ? COLOR_MAGENTA : COLOR_BORDER);
    FillRect(hdc, &portsRect, portsBrush);
    DeleteObject(portsBrush);
    SetTextColor(hdc, data->view_mode == VIEW_MODE_PORTS ? RGB(0, 0, 0) : COLOR_TEXT_MUTED);
    DrawText(hdc, modePorts, -1, &portsRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    y += 22;

    // ===== CHAMP DE RECHERCHE =====
    // Fond du champ de recherche
    RECT searchRect = {16, y, width - 16, y + 20};
    HBRUSH searchBg = CreateSolidBrush(data->filter_active ? RGB(40, 40, 50) : RGB(25, 25, 35));
    FillRect(hdc, &searchRect, searchBg);
    DeleteObject(searchBg);

    // Bordure du champ
    HPEN searchPen = CreatePen(PS_SOLID, 1, data->filter_active ? COLOR_CYAN : COLOR_BORDER);
    SelectObject(hdc, searchPen);
    MoveToEx(hdc, searchRect.left, searchRect.top, NULL);
    LineTo(hdc, searchRect.right, searchRect.top);
    LineTo(hdc, searchRect.right, searchRect.bottom);
    LineTo(hdc, searchRect.left, searchRect.bottom);
    LineTo(hdc, searchRect.left, searchRect.top);
    DeleteObject(searchPen);

    // Icone loupe
    SetTextColor(hdc, COLOR_TEXT_MUTED);
    TextOut(hdc, 20, y + 3, ">", 1);

    // Texte du filtre ou placeholder
    if (data->filter_text[0] != '\0') {
        SetTextColor(hdc, COLOR_TEXT_PRIMARY);
        TextOut(hdc, 32, y + 3, data->filter_text, (int)strlen(data->filter_text));
        if (data->filter_active) {
            int cursorX = 32 + (int)strlen(data->filter_text) * 7;
            SetTextColor(hdc, COLOR_CYAN);
            TextOut(hdc, cursorX, y + 3, "_", 1);
        }
    } else {
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        TextOut(hdc, 32, y + 3, "Filter...", 9);
    }

    // Bouton clear si filtre actif
    if (data->filter_text[0] != '\0') {
        SetTextColor(hdc, COLOR_KILL_BUTTON);
        TextOut(hdc, width - 30, y + 3, "X", 1);
    }

    y += 24;

    // Separation
    HPEN sepPen = CreatePen(PS_SOLID, 1, COLOR_MAGENTA);
    SelectObject(hdc, sepPen);
    MoveToEx(hdc, 14, y, NULL);
    LineTo(hdc, width - 14, y);
    DeleteObject(sepPen);
    y += 6;

    // Positions des colonnes selon le mode
    const int COL_INDICATOR = 16;
    int COL_MEM_OR_PORT, COL_PROCESS, COL_PID;

    if (data->view_mode == VIEW_MODE_ALL) {
        COL_MEM_OR_PORT = 28;   // Colonne RAM
        COL_PROCESS = 80;
        COL_PID = 190;
    } else {
        COL_MEM_OR_PORT = 28;   // Colonne PORT
        COL_PROCESS = 90;
        COL_PID = 200;
    }

    // Constantes scrollbar
    const int SCROLLBAR_WIDTH = 8;
    const int SCROLLBAR_X = width - 12;
    const int LIST_START_Y = y + 16;
    const int maxVisible = 6;  // Reduit pour le toggle
    const int LIST_HEIGHT = maxVisible * PROCESS_LINE_HEIGHT;

    // Nombre de processus filtrés
    int filteredCount = GetFilteredProcessCount(data);

    // En-tete
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COLOR_TEXT_MUTED);
    if (data->view_mode == VIEW_MODE_ALL) {
        TextOut(hdc, COL_MEM_OR_PORT, y, "RAM", 3);
    } else {
        TextOut(hdc, COL_MEM_OR_PORT, y, "PORT", 4);
    }
    TextOut(hdc, COL_PROCESS, y, "PROCESS", 7);
    TextOut(hdc, COL_PID, y, "PID", 3);
    y += 18;

    // Liste des processus (filtrés)
    SelectObject(hdc, hFontNormal);
    int startIdx = data->scroll_offset;
    int endIdx = startIdx + maxVisible;
    if (endIdx > filteredCount) endIdx = filteredCount;

    for (int visibleIdx = startIdx; visibleIdx < endIdx; visibleIdx++) {
        ProcessInfo* proc = GetFilteredProcessByVisibleIndex(data, visibleIdx);
        if (proc == NULL) continue;

        // Indicateur actif/critique
        if (proc->is_critical) {
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            TextOut(hdc, COL_INDICATOR, y, "!", 1);
        } else {
            SetTextColor(hdc, proc->is_active ? COLOR_PROCESS_ACTIVE : COLOR_PROCESS_INACTIVE);
            TextOut(hdc, COL_INDICATOR, y, "*", 1);
        }

        // RAM ou Port selon le mode
        char line[64];
        if (data->view_mode == VIEW_MODE_ALL) {
            snprintf(line, sizeof(line), "%luM", (unsigned long)proc->memory_mb);
            SetTextColor(hdc, COLOR_CYAN);
        } else {
            snprintf(line, sizeof(line), ":%d", proc->port);
            SetTextColor(hdc, COLOR_CYAN);
        }
        TextOut(hdc, COL_MEM_OR_PORT, y, line, (int)strlen(line));

        // Nom du processus (tronque si trop long)
        char procName[20];
        int maxLen = (data->view_mode == VIEW_MODE_ALL) ? 14 : 12;
        strncpy(procName, proc->name, maxLen);
        procName[maxLen] = '\0';
        if ((int)strlen(proc->name) > maxLen) strcat(procName, "..");
        SetTextColor(hdc, proc->is_critical ? COLOR_TEXT_MUTED : COLOR_TEXT_PRIMARY);
        TextOut(hdc, COL_PROCESS, y, procName, (int)strlen(procName));

        // PID
        snprintf(line, sizeof(line), "%lu", proc->pid);
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        TextOut(hdc, COL_PID, y, line, (int)strlen(line));

        // Bouton Kill (grise si critique)
        RECT killRect = {width - 70, y - 2, width - 26, y + 16};
        HBRUSH killBrush = CreateSolidBrush(proc->is_critical ? COLOR_BORDER : COLOR_KILL_BUTTON);
        FillRect(hdc, &killRect, killBrush);
        DeleteObject(killBrush);
        SetTextColor(hdc, proc->is_critical ? COLOR_TEXT_MUTED : RGB(255, 255, 255));
        SelectObject(hdc, hFontSmall);
        DrawText(hdc, "Kill", -1, &killRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hFontNormal);

        y += PROCESS_LINE_HEIGHT;
    }

    // ===== SCROLLBAR LATERALE =====
    if (filteredCount > maxVisible) {
        // Fond de la scrollbar (track)
        RECT trackRect = {SCROLLBAR_X - SCROLLBAR_WIDTH/2, LIST_START_Y,
                          SCROLLBAR_X + SCROLLBAR_WIDTH/2, LIST_START_Y + LIST_HEIGHT};
        HBRUSH trackBrush = CreateSolidBrush(COLOR_BORDER);
        FillRect(hdc, &trackRect, trackBrush);
        DeleteObject(trackBrush);

        // Calculer la taille et position du thumb
        int thumbHeight = (LIST_HEIGHT * maxVisible) / filteredCount;
        if (thumbHeight < 20) thumbHeight = 20;

        int maxScroll = filteredCount - maxVisible;
        int thumbY = LIST_START_Y;
        if (maxScroll > 0) {
            thumbY = LIST_START_Y + (data->scroll_offset * (LIST_HEIGHT - thumbHeight)) / maxScroll;
        }

        // Dessiner le thumb
        RECT thumbRect = {SCROLLBAR_X - SCROLLBAR_WIDTH/2, thumbY,
                          SCROLLBAR_X + SCROLLBAR_WIDTH/2, thumbY + thumbHeight};
        HBRUSH thumbBrush = CreateSolidBrush(COLOR_MAGENTA);
        FillRect(hdc, &thumbRect, thumbBrush);
        DeleteObject(thumbBrush);
    }

    // Message si aucun processus ou aucun résultat
    if (filteredCount == 0) {
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        if (data->filter_text[0] != '\0') {
            TextOut(hdc, 16, y, "No matching processes", 21);
        } else {
            TextOut(hdc, 16, y, "No processes with open ports", 28);
        }
        y += 20;
    }

    // Boutons en bas
    y = height - 40;

    // Bouton Refresh
    RECT refreshRect = {16, y, 100, y + 24};
    HBRUSH refreshBrush = CreateSolidBrush(COLOR_CYAN);
    FillRect(hdc, &refreshRect, refreshBrush);
    DeleteObject(refreshBrush);
    SetTextColor(hdc, RGB(0, 0, 0));
    SelectObject(hdc, hFontSmall);
    DrawText(hdc, "Refresh", -1, &refreshRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // Info count + scroll info
    char countStr[64];
    if (data->filter_text[0] != '\0') {
        snprintf(countStr, sizeof(countStr), "%d/%d filtered", filteredCount, data->count);
    } else if (filteredCount > maxVisible) {
        snprintf(countStr, sizeof(countStr), "%d processes (%d-%d)",
                 filteredCount, data->scroll_offset + 1,
                 (data->scroll_offset + maxVisible > filteredCount) ? filteredCount : data->scroll_offset + maxVisible);
    } else {
        snprintf(countStr, sizeof(countStr), "%d processes", data->count);
    }
    SetTextColor(hdc, COLOR_TEXT_MUTED);
    TextOut(hdc, 120, y + 5, countStr, (int)strlen(countStr));
}

/*
 * NOTE: Les fonctions BuildBarString, FormatUptime et BuildDiskLine
 * sont maintenant dans les plugins respectifs (v3.0)
 */

/*
 * InitBanner
 * Prepare les lignes d'entete (logo) en recuperant le nom utilisateur si possible
 */
static void InitBanner() {
    char user[64] = {0};
    DWORD sz = (DWORD)sizeof(user);
    if (GetUserNameA(user, &sz) && sz > 1) {
        snprintf(g_bannerTop, sizeof(g_bannerTop), "==[ %s OVERLAY ]==", "SYSTEM");
        snprintf(g_bannerBottom, sizeof(g_bannerBottom), ":: %s @ overlay ::", user);
    } else {
        snprintf(g_bannerTop, sizeof(g_bannerTop), "==[ SYSTEM OVERLAY ]==");
        snprintf(g_bannerBottom, sizeof(g_bannerBottom), ":: PERF OVERLAY v2 ::");
    }
}

/*
 * UpdateDisplay
 * Met a jour l'affichage des informations de performance
 */
void UpdateDisplay() {
    InvalidateRect(g_hwnd, NULL, TRUE);
}

/*
 * CreateTrayIcon
 * Crée l'icône dans le system tray
 */
static void CreateTrayIcon(HWND hwnd, HINSTANCE hInstance) {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAYICON;
    g_nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(1));
    if (!g_nid.hIcon) {
        g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    strcpy(g_nid.szTip, "Performance Overlay v2");

    Shell_NotifyIcon(NIM_ADD, &g_nid);
    g_trayIconAdded = TRUE;
}

/*
 * RemoveTrayIcon
 * Supprime l'icône du system tray
 */
static void RemoveTrayIcon(void) {
    if (g_trayIconAdded) {
        Shell_NotifyIcon(NIM_DELETE, &g_nid);
        g_trayIconAdded = FALSE;
    }
}

/*
 * ShowTrayMenu
 * Affiche le menu contextuel du system tray
 */
static void ShowTrayMenu(HWND hwnd) {
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
    AppendMenu(hMenu, MF_STRING, ID_TRAY_SETTINGS, "Paramètres...");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, "Quitter");

    // Nécessaire pour que le menu se ferme correctement
    SetForegroundWindow(hwnd);

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

    DestroyMenu(hMenu);
}

/*
 * WindowProc
 * Fonction de traitement des messages de la fenetre
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
            // Créer l'icône system tray
            HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
            CreateTrayIcon(hwnd, hInst);
            return 0;
        }

        case WM_TRAYICON:
            // Gestion des clics sur l'icône du tray
            if (lParam == WM_RBUTTONUP) {
                ShowTrayMenu(hwnd);
            } else if (lParam == WM_LBUTTONDBLCLK) {
                // Double-clic gauche : afficher/masquer
                ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
                if (IsWindowVisible(hwnd)) {
                    SetForegroundWindow(hwnd);
                }
            }
            return 0;

        case WM_COMMAND:
            // Gestion des commandes du menu tray
            switch (LOWORD(wParam)) {
                case ID_TRAY_SHOW:
                    ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
                    if (IsWindowVisible(hwnd)) {
                        SetForegroundWindow(hwnd);
                    }
                    break;
                case ID_TRAY_SETTINGS:
                    // Ouvrir la page Settings
                    g_currentPage = PAGE_SETTINGS;
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, WINDOW_WIDTH,
                                GetWindowHeightForMode(g_config.minimal_mode),
                                SWP_NOMOVE | SWP_NOZORDER);
                    ShowWindow(hwnd, SW_SHOW);
                    SetForegroundWindow(hwnd);
                    UpdateDisplay();
                    break;
                case ID_TRAY_EXIT:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;

        case WM_TIMER:
            // Mettre à jour tous les plugins (v3.0)
            UpdateAllPlugins();
            UpdateDisplay();
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right;
            int height = clientRect.bottom;

            // Fond
            HBRUSH bgBrush = CreateSolidBrush(g_colorBg);
            FillRect(hdc, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            RECT panelRect = {6, 8, width - 6, height - 6};
            HBRUSH panelBrush = CreateSolidBrush(g_colorPanel);
            FillRect(hdc, &panelRect, panelBrush);
            DeleteObject(panelBrush);

            // Bordure
            HPEN borderPen = CreatePen(PS_SOLID, 1, g_colorBorder);
            SelectObject(hdc, borderPen);
            MoveToEx(hdc, panelRect.left, panelRect.top, NULL);
            LineTo(hdc, panelRect.right, panelRect.top);
            LineTo(hdc, panelRect.right, panelRect.bottom);
            LineTo(hdc, panelRect.left, panelRect.bottom);
            LineTo(hdc, panelRect.left, panelRect.top);
            DeleteObject(borderPen);

            // Accent haut
            HBRUSH accent = CreateSolidBrush(g_colorAccent2);
            RECT accentRect = {panelRect.left, panelRect.top, panelRect.right, panelRect.top + 4};
            FillRect(hdc, &accentRect, accent);
            DeleteObject(accent);

            // Bouton X (zone clickable deja geree)
            RECT closeRect = {width - 28, 10, width - 6, 28};
            HBRUSH closeBrush = CreateSolidBrush(g_colorAccent2);
            FillRect(hdc, &closeRect, closeBrush);
            DeleteObject(closeBrush);

            // Polices
            HFONT hFontTitle = CreateFont(FONT_TITLE_SIZE, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, FONT_TITLE_NAME);
            HFONT hFontNormal = CreateFont(FONT_NORMAL_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, FONT_NORMAL_NAME);
            HFONT hFontSmall = CreateFont(FONT_SMALL_SIZE, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, FONT_SMALL_NAME);

            SetBkMode(hdc, TRANSPARENT);

            // Dessiner les onglets
            DrawTabs(hdc, width, hFontSmall);

            // Bouton X texte
            SetTextColor(hdc, RGB(0, 0, 0));
            SelectObject(hdc, hFontSmall);
            TextOut(hdc, closeRect.left + 5, closeRect.top + 2, "X", 1);

            // Contenu selon la page active
            if (g_currentPage == PAGE_TASKS) {
                // Page Task Killer
                DrawTaskKillerPage(hdc, width, height, hFontNormal, hFontSmall);
            } else if (g_currentPage == PAGE_SETTINGS) {
                // Page Settings/Skins
                DrawSettingsPage(hdc, width, height, hFontNormal, hFontSmall);
            } else {
                // Page Performance (par defaut)
                int y = 50;  // Apres les onglets

                // Logo ASCII
                SelectObject(hdc, hFontTitle);
                SetTextColor(hdc, g_colorAccent);
                TextOut(hdc, 16, y, g_bannerTop, (int)strlen(g_bannerTop));
                y += 18;
                SelectObject(hdc, hFontNormal);
                SetTextColor(hdc, g_colorAccent2);
                TextOut(hdc, 16, y, g_bannerBottom, (int)strlen(g_bannerBottom));
                y += 16;

                // Afficher Uptime d'abord (si activé)
                MetricData* uptime = GetMetricByName("Uptime");
                if (uptime && uptime->enabled && g_config.show_uptime) {
                    SetTextColor(hdc, uptime->color);
                    TextOut(hdc, 16, y, uptime->display_lines[0], (int)strlen(uptime->display_lines[0]));
                    y += 12;
                }

                // Afficher Prayer juste après Uptime
                MetricData* prayer = GetMetricByName("Prayer");
                if (prayer && prayer->enabled && g_config.prayer_enabled) {
                    SetTextColor(hdc, prayer->color);
                    TextOut(hdc, 16, y, prayer->display_lines[0], (int)strlen(prayer->display_lines[0]));
                    y += 14;
                }

                // Separation
                HPEN sepPen = CreatePen(PS_SOLID, 1, g_colorAccent2);
                SelectObject(hdc, sepPen);
                MoveToEx(hdc, 14, y + 6, NULL);
                LineTo(hdc, width - 14, y + 6);
                DeleteObject(sepPen);
                y += 14;

                // Afficher les métriques via le système de plugins (v3.0)
                const char* pluginOrder[] = {"CPU", "RAM", "GPU", "Disk", "Process", "Volume", "DateTime"};
                int pluginCount = 7;

                for (int i = 0; i < pluginCount; i++) {
                    MetricData* metric = GetMetricByName(pluginOrder[i]);
                    if (metric && metric->enabled) {
                        if (g_config.minimal_mode && strcmp(pluginOrder[i], "Disk") == 0) {
                            continue;
                        }
                        if (g_config.minimal_mode && strcmp(pluginOrder[i], "Process") == 0) {
                            continue;
                        }
                        if (!g_config.show_processes && strcmp(pluginOrder[i], "Process") == 0) {
                            continue;
                        }

                        SetTextColor(hdc, metric->color);
                        for (int line = 0; line < metric->line_count; line++) {
                            TextOut(hdc, 16, y, metric->display_lines[line],
                                    (int)strlen(metric->display_lines[line]));
                            y += 18;
                        }
                    }
                }

                // Footer
                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, g_colorTextMuted);
                const char* footer = "F2 minimal | F4 tasks";
                TextOut(hdc, 16, height - 26, footer, (int)strlen(footer));
            }

            DeleteObject(hFontTitle);
            DeleteObject(hFontNormal);
            DeleteObject(hFontSmall);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right;
            int height = clientRect.bottom;

            // Bouton X (fermer)
            if (pt.x >= width - 28 && pt.x <= width - 6 && pt.y >= 10 && pt.y <= 28) {
                PostQuitMessage(0);
                return 0;
            }

            // Clic sur les onglets
            int tabX = 16;
            int tabY = 12;
            for (int i = 0; i < PAGE_COUNT; i++) {
                if (pt.x >= tabX && pt.x <= tabX + TAB_WIDTH &&
                    pt.y >= tabY && pt.y <= tabY + TAB_HEIGHT) {
                    if (g_currentPage != i) {
                        g_currentPage = i;
                        // Redimensionner la fenetre selon la page
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, WINDOW_WIDTH,
                                    GetWindowHeightForMode(g_config.minimal_mode),
                                    SWP_NOMOVE | SWP_NOZORDER);
                        // Rafraichir les donnees si Task Killer
                        if (g_currentPage == PAGE_TASKS) {
                            RefreshCurrentView(GetTaskKillerData());
                        }
                        UpdateDisplay();
                    }
                    return 0;
                }
                tabX += TAB_WIDTH + TAB_MARGIN;
            }

            // Clics specifiques a la page Task Killer
            if (g_currentPage == PAGE_TASKS) {
                TaskKillerData* data = GetTaskKillerData();

                // Constantes (doivent correspondre a DrawTaskKillerPage)
                const int TOGGLE_Y = 50;
                const int TOGGLE_HEIGHT = 18;
                const int SEARCH_Y = 72;  // 50 + 22
                const int SEARCH_HEIGHT = 20;
                const int SCROLLBAR_WIDTH = 8;
                const int SCROLLBAR_X = width - 12;
                const int LIST_START_Y = 118;  // 50 + 22 + 24 + 6 + 16
                const int maxVisible = 6;
                const int LIST_HEIGHT = maxVisible * PROCESS_LINE_HEIGHT;
                int filteredCount = GetFilteredProcessCount(data);

                // Clic sur toggle "All"
                if (pt.x >= 16 && pt.x <= 56 &&
                    pt.y >= TOGGLE_Y && pt.y <= TOGGLE_Y + TOGGLE_HEIGHT) {
                    if (data->view_mode != VIEW_MODE_ALL) {
                        data->view_mode = VIEW_MODE_ALL;
                        data->scroll_offset = 0;
                        RefreshCurrentView(data);
                        UpdateDisplay();
                    }
                    return 0;
                }

                // Clic sur toggle "Ports"
                if (pt.x >= 58 && pt.x <= 108 &&
                    pt.y >= TOGGLE_Y && pt.y <= TOGGLE_Y + TOGGLE_HEIGHT) {
                    if (data->view_mode != VIEW_MODE_PORTS) {
                        data->view_mode = VIEW_MODE_PORTS;
                        data->scroll_offset = 0;
                        RefreshCurrentView(data);
                        UpdateDisplay();
                    }
                    return 0;
                }

                // Clic sur le champ de recherche
                if (pt.x >= 16 && pt.x <= width - 16 &&
                    pt.y >= SEARCH_Y && pt.y <= SEARCH_Y + SEARCH_HEIGHT) {
                    // Clic sur le bouton X (clear filter)
                    if (pt.x >= width - 35 && data->filter_text[0] != '\0') {
                        data->filter_text[0] = '\0';
                        data->scroll_offset = 0;
                    }
                    data->filter_active = TRUE;
                    UpdateDisplay();
                    return 0;
                } else {
                    // Clic ailleurs desactive le champ
                    if (data->filter_active) {
                        data->filter_active = FALSE;
                        UpdateDisplay();
                    }
                }

                // Bouton Refresh
                if (pt.x >= 16 && pt.x <= 100 &&
                    pt.y >= height - 40 && pt.y <= height - 16) {
                    RefreshCurrentView(data);
                    data->scroll_offset = 0;
                    UpdateDisplay();
                    return 0;
                }

                // Clic sur la scrollbar
                if (filteredCount > maxVisible &&
                    pt.x >= SCROLLBAR_X - SCROLLBAR_WIDTH &&
                    pt.x <= SCROLLBAR_X + SCROLLBAR_WIDTH &&
                    pt.y >= LIST_START_Y && pt.y <= LIST_START_Y + LIST_HEIGHT) {
                    // Calculer la nouvelle position de scroll
                    int clickPos = pt.y - LIST_START_Y;
                    int maxScroll = filteredCount - maxVisible;
                    int newOffset = (clickPos * maxScroll) / LIST_HEIGHT;
                    if (newOffset < 0) newOffset = 0;
                    if (newOffset > maxScroll) newOffset = maxScroll;
                    data->scroll_offset = newOffset;
                    UpdateDisplay();
                    return 0;
                }

                // Boutons Kill pour chaque processus (utilise l'index filtre)
                int y = LIST_START_Y;
                int startIdx = data->scroll_offset;
                int endIdx = startIdx + maxVisible;
                if (endIdx > filteredCount) endIdx = filteredCount;

                for (int visibleIdx = startIdx; visibleIdx < endIdx; visibleIdx++) {
                    if (pt.x >= width - 70 && pt.x <= width - 26 &&
                        pt.y >= y - 2 && pt.y <= y + 16) {
                        // Convertir l'index visible en index reel
                        int realIdx = GetRealIndexFromVisibleIndex(data, visibleIdx);
                        if (realIdx >= 0) {
                            if (KillProcessByIndex(data, realIdx)) {
                                UpdateDisplay();
                            }
                        }
                        return 0;
                    }
                    y += PROCESS_LINE_HEIGHT;
                }
            }

            // Clics spécifiques à la page Settings
            if (g_currentPage == PAGE_SETTINGS) {
                int y = 80;  // Position de départ des thèmes
                for (int i = 0; i < g_themeCount; i++) {
                    if (pt.x >= 16 && pt.x <= width - 16 &&
                        pt.y >= y && pt.y <= y + 28) {
                        // Sélectionner et appliquer ce thème
                        ApplyTheme(i);
                        g_config.theme_index = i;
                        SaveConfigINI(&g_config, CONFIG_FILE_INI);
                        UpdateDisplay();
                        return 0;
                    }
                    y += 32;
                }
            }

            // Drag de la fenetre
            g_isDragging = TRUE;
            g_dragOffset.x = pt.x;
            g_dragOffset.y = pt.y;
            SetCapture(hwnd);
            return 0;
        }

        case WM_LBUTTONUP:
            if (g_isDragging) {
                g_isDragging = FALSE;
                ReleaseCapture();

                RECT rect;
                GetWindowRect(hwnd, &rect);
                g_config.x = rect.left;
                g_config.y = rect.top;
                SaveConfigINI(&g_config, CONFIG_FILE_INI);
            }
            return 0;

        case WM_MOUSEMOVE:
            if (g_isDragging) {
                POINT pt;
                GetCursorPos(&pt);
                SetWindowPos(hwnd, HWND_TOPMOST, pt.x - g_dragOffset.x, pt.y - g_dragOffset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
            }
            return 0;

        case WM_MOUSEWHEEL: {
            // Gestion de la molette pour le scroll dans Task Killer
            if (g_currentPage == PAGE_TASKS) {
                TaskKillerData* data = GetTaskKillerData();
                const int maxVisible = 6;
                int filteredCount = GetFilteredProcessCount(data);

                if (filteredCount > maxVisible) {
                    int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                    int scrollLines = delta / WHEEL_DELTA;

                    data->scroll_offset -= scrollLines;

                    int maxScroll = filteredCount - maxVisible;
                    if (data->scroll_offset < 0) data->scroll_offset = 0;
                    if (data->scroll_offset > maxScroll) data->scroll_offset = maxScroll;

                    UpdateDisplay();
                }
            }
            return 0;
        }

        case WM_CHAR: {
            // Gestion de la saisie pour le filtre Task Killer
            if (g_currentPage == PAGE_TASKS) {
                TaskKillerData* data = GetTaskKillerData();

                if (data->filter_active) {
                    char c = (char)wParam;
                    int len = (int)strlen(data->filter_text);

                    if (c == '\b') {
                        // Backspace - supprimer dernier caractere
                        if (len > 0) {
                            data->filter_text[len - 1] = '\0';
                            data->scroll_offset = 0;
                            UpdateDisplay();
                        }
                    } else if (c == 27) {
                        // Escape - desactiver le filtre
                        data->filter_active = FALSE;
                        UpdateDisplay();
                    } else if (c >= 32 && c < 127 && len < 30) {
                        // Caractere imprimable
                        data->filter_text[len] = c;
                        data->filter_text[len + 1] = '\0';
                        data->scroll_offset = 0;
                        UpdateDisplay();
                    }
                    return 0;
                }
            }
            break;
        }

        case WM_KEYDOWN:
            // Utiliser les touches configurables (v3.0)
            if (wParam == g_config.toggle_minimal_key) {
                // F2 - Mode minimal (seulement sur page Perf)
                if (g_currentPage == PAGE_PERF) {
                    g_config.minimal_mode = !g_config.minimal_mode;
                    SaveConfigINI(&g_config, CONFIG_FILE_INI);
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, WINDOW_WIDTH,
                                GetWindowHeightForMode(g_config.minimal_mode),
                                SWP_NOMOVE | SWP_NOZORDER);
                    UpdateDisplay();
                }
            } else if (wParam == VK_F4) {
                // F4 - Basculer entre pages Perf et Tasks
                g_currentPage = (g_currentPage == PAGE_PERF) ? PAGE_TASKS : PAGE_PERF;
                // Rafraichir si on va sur Task Killer
                if (g_currentPage == PAGE_TASKS) {
                    RefreshProcessList(GetTaskKillerData());
                }
                // Redimensionner la fenetre
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, WINDOW_WIDTH,
                            GetWindowHeightForMode(g_config.minimal_mode),
                            SWP_NOMOVE | SWP_NOZORDER);
                UpdateDisplay();
            } else if (wParam == g_config.reload_config_key) {
                // Recharger la configuration (nouveau en v3.0)
                LoadConfigINI(&g_config, CONFIG_FILE_INI);
                UpdateDisplay();
            }
            return 0;

        case WM_DESTROY:
            RemoveTrayIcon();       // Supprimer l'icône du tray
            KillTimer(hwnd, TIMER_ID);
            CleanupTaskKiller();    // Nettoyer le Task Killer
            CleanupPluginSystem();  // Nettoyer les plugins (v3.0)
            CleanupPerformanceMonitoring();
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
 * CreateOverlayWindow
 * Cree la fenetre de l'overlay
 */
HWND CreateOverlayWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

    RegisterClass(&wc);

    // Utiliser config.txt MAIS valider la position
    // Si hors écran, centrer sur écran principal
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int posX = g_config.x;
    int posY = g_config.y;

    // Validation : si position semble invalide, centrer
    if (posX < 0 || posX > screenWidth - 100 || posY < 0 || posY > screenHeight - 100) {
        posX = (screenWidth - WINDOW_WIDTH) / 2;
        posY = (screenHeight - GetWindowHeightForMode(g_config.minimal_mode)) / 2;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,  // Style overlay complet
        WINDOW_CLASS_NAME,
        "Performance Overlay",
        WS_POPUP,  // Style popup (sans bordures)
        posX,
        posY,
        WINDOW_WIDTH,
        GetWindowHeightForMode(g_config.minimal_mode),
        NULL,
        NULL,
        hInstance,
        NULL
    );

    SetLayeredWindowAttributes(hwnd, 0, WINDOW_OPACITY, LWA_ALPHA);
    return hwnd;
}

/*
 * WinMain
 * Point d'entree du programme Windows
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

    // Initialiser le monitoring legacy (fallback)
    InitPerformanceMonitoring();

    // Charger la configuration INI (v3.0)
    LoadConfigINI(&g_config, CONFIG_FILE_INI);

    // Appliquer le thème depuis la config
    ApplyTheme(g_config.theme_index);

    // Initialiser le système de plugins
    InitPluginSystem();
    RegisterMetricPlugin(&CPUPlugin);
    RegisterMetricPlugin(&RAMPlugin);
    RegisterMetricPlugin(&DiskPlugin);
    RegisterMetricPlugin(&UptimePlugin);
    RegisterMetricPlugin(&ProcessPlugin);
    RegisterMetricPlugin(&NetworkPlugin);
    RegisterMetricPlugin(&DateTimePlugin);
    RegisterMetricPlugin(&VolumePlugin);
    RegisterMetricPlugin(&GPUPlugin);
    RegisterMetricPlugin(&PrayerPlugin);

    // Configurer les horaires de prière depuis la config (avec API)
    SetPrayerConfig(g_config.prayer_city, g_config.prayer_country,
                    g_config.prayer_method, g_config.prayer_use_api,
                    g_config.prayer_enabled,
                    g_config.prayer_fajr, g_config.prayer_dhuhr,
                    g_config.prayer_asr, g_config.prayer_maghrib,
                    g_config.prayer_isha);

    // Activer/désactiver plugins selon config
    EnablePlugin("CPU", g_config.cpu_enabled);
    EnablePlugin("RAM", g_config.ram_enabled);
    EnablePlugin("Disk", g_config.disk_enabled);
    EnablePlugin("Uptime", g_config.uptime_enabled);
    EnablePlugin("Process", g_config.process_enabled);
    EnablePlugin("Prayer", g_config.prayer_enabled);
    // Nouveaux plugins - activés par défaut
    EnablePlugin("Network", TRUE);
    EnablePlugin("DateTime", TRUE);
    EnablePlugin("Volume", TRUE);
    EnablePlugin("GPU", TRUE);

    // Initialiser le Task Killer
    InitTaskKiller();

    InitBanner();
    AddToStartup();

    // Première mise à jour des plugins
    UpdateAllPlugins();

    g_hwnd = CreateOverlayWindow(hInstance);
    if (g_hwnd == NULL) {
        MessageBox(NULL, "Erreur de creation de la fenetre!", "Erreur", MB_ICONERROR);
        return 1;
    }

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Le cleanup est fait dans WM_DESTROY
    return 0;
}
