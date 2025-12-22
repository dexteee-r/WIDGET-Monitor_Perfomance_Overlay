/*
 * main.c
 * Programme principal - Overlay de performance systeme
 *
 * Overlay temps reel minimaliste (style cyberdeck/TUI)
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../include/performance.h"
#include "../include/config.h"
#include "../include/config_parser.h"
#include "../include/startup.h"
#include "../include/constants.h"
#include "../include/metric_plugin.h"
#include "../include/all_plugins.h"

// Variables globales
HWND g_hwnd = NULL;              // Handle de la fenetre principale
ConfigINI g_config;              // Configuration de l'application (v3.0 - INI)
Config g_configLegacy;           // Configuration legacy (v2.0 - TXT) pour compatibilite
PerformanceData g_perfData;      // Donnees de performance (legacy, pour fallback)
BOOL g_isDragging = FALSE;       // Etat du deplacement de fenetre
POINT g_dragOffset;              // Offset lors du drag
char g_bannerTop[64] = "==[ SYSTEM OVERLAY v3 ]==";
char g_bannerBottom[64] = ":: PLUGIN SYSTEM ::";

static int GetWindowHeightForMode(BOOL minimal) {
    return minimal ? WINDOW_HEIGHT_MIN : WINDOW_HEIGHT_FULL;
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
 * WindowProc
 * Fonction de traitement des messages de la fenetre
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
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
            HBRUSH bgBrush = CreateSolidBrush(COLOR_BG);
            FillRect(hdc, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            RECT panelRect = {6, 8, width - 6, height - 6};
            HBRUSH panelBrush = CreateSolidBrush(COLOR_PANEL);
            FillRect(hdc, &panelRect, panelBrush);
            DeleteObject(panelBrush);

            // Bordure
            HPEN borderPen = CreatePen(PS_SOLID, 1, COLOR_BORDER);
            SelectObject(hdc, borderPen);
            MoveToEx(hdc, panelRect.left, panelRect.top, NULL);
            LineTo(hdc, panelRect.right, panelRect.top);
            LineTo(hdc, panelRect.right, panelRect.bottom);
            LineTo(hdc, panelRect.left, panelRect.bottom);
            LineTo(hdc, panelRect.left, panelRect.top);
            DeleteObject(borderPen);

            // Accent haut
            HBRUSH accent = CreateSolidBrush(COLOR_MAGENTA);
            RECT accentRect = {panelRect.left, panelRect.top, panelRect.right, panelRect.top + 4};
            FillRect(hdc, &accentRect, accent);
            DeleteObject(accent);

            // Bouton X (zone clickable deja geree)
            RECT closeRect = {width - 28, 10, width - 6, 28};
            HBRUSH closeBrush = CreateSolidBrush(COLOR_MAGENTA);
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

            // Logo ASCII
            int y = 14;
            SelectObject(hdc, hFontTitle);
            SetTextColor(hdc, COLOR_CYAN);
            TextOut(hdc, 16, y, g_bannerTop, (int)strlen(g_bannerTop));
            y += 18;
            SelectObject(hdc, hFontNormal);
            SetTextColor(hdc, COLOR_MAGENTA);
            TextOut(hdc, 16, y, g_bannerBottom, (int)strlen(g_bannerBottom));
            y += 16;

            // Afficher Uptime d'abord (si activé)
            MetricData* uptime = GetMetricByName("Uptime");
            if (uptime && uptime->enabled && g_config.show_uptime) {
                SetTextColor(hdc, uptime->color);
                TextOut(hdc, 16, y, uptime->display_lines[0], (int)strlen(uptime->display_lines[0]));
                y += 12;
            }

            // Separation
            HPEN sepPen = CreatePen(PS_SOLID, 1, COLOR_MAGENTA);
            SelectObject(hdc, sepPen);
            MoveToEx(hdc, 14, y + 6, NULL);
            LineTo(hdc, width - 14, y + 6);
            DeleteObject(sepPen);
            y += 14;

            // Afficher les métriques via le système de plugins (v3.0)
            // Ordre: CPU, RAM, Disk, Process
            const char* pluginOrder[] = {"CPU", "RAM", "Disk", "Process"};
            int pluginCount = 4;

            for (int i = 0; i < pluginCount; i++) {
                MetricData* metric = GetMetricByName(pluginOrder[i]);
                if (metric && metric->enabled) {
                    // Vérifier si on doit afficher selon le mode
                    if (g_config.minimal_mode && strcmp(pluginOrder[i], "Disk") == 0) {
                        continue;  // Pas de disques en mode minimal
                    }
                    if (g_config.minimal_mode && strcmp(pluginOrder[i], "Process") == 0) {
                        continue;  // Pas de processus en mode minimal
                    }
                    if (!g_config.show_processes && strcmp(pluginOrder[i], "Process") == 0) {
                        continue;  // Processus désactivé dans config
                    }

                    // Afficher chaque ligne de la métrique
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
            SetTextColor(hdc, COLOR_TEXT_MUTED);
            const char* footer = "F2 minimal  |  F3 hide/show";
            TextOut(hdc, 16, height - 26, footer, (int)strlen(footer));

            // Bouton X texte
            SetTextColor(hdc, RGB(0, 0, 0));
            TextOut(hdc, closeRect.left + 5, closeRect.top + 2, "X", 1);

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

            if (pt.x >= width - 28 && pt.x <= width - 6 && pt.y >= 10 && pt.y <= 28) {
                PostQuitMessage(0);
                return 0;
            }

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

        case WM_KEYDOWN:
            // Utiliser les touches configurables (v3.0)
            if (wParam == g_config.toggle_visibility_key) {
                ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
            } else if (wParam == g_config.toggle_minimal_key) {
                g_config.minimal_mode = !g_config.minimal_mode;
                SaveConfigINI(&g_config, CONFIG_FILE_INI);
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
            KillTimer(hwnd, TIMER_ID);
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

    // Initialiser le système de plugins
    InitPluginSystem();
    RegisterMetricPlugin(&CPUPlugin);
    RegisterMetricPlugin(&RAMPlugin);
    RegisterMetricPlugin(&DiskPlugin);
    RegisterMetricPlugin(&UptimePlugin);
    RegisterMetricPlugin(&ProcessPlugin);

    // Activer/désactiver plugins selon config
    EnablePlugin("CPU", g_config.cpu_enabled);
    EnablePlugin("RAM", g_config.ram_enabled);
    EnablePlugin("Disk", g_config.disk_enabled);
    EnablePlugin("Uptime", g_config.uptime_enabled);
    EnablePlugin("Process", g_config.process_enabled);

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
