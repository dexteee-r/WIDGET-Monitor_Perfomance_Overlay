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
int g_currentPage = PAGE_PERF;   // Page actuelle (0=Perf, 1=Tasks, 2=Prayer)
const char* g_pageNames[] = {"Perf", "Tasks", "Prayer"};

static int GetWindowHeightForMode(BOOL minimal) {
    if (g_currentPage == PAGE_TASKS) {
        return WINDOW_HEIGHT_TASKS;
    }
    return minimal ? WINDOW_HEIGHT_MIN : WINDOW_HEIGHT_FULL;
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
        // Si page Prayer pas encore implementee, on la cache
        if (i == PAGE_PRAYER) continue;

        RECT tabRect = {tabX, tabY, tabX + TAB_WIDTH, tabY + TAB_HEIGHT};

        // Fond de l'onglet
        HBRUSH tabBrush;
        if (i == g_currentPage) {
            tabBrush = CreateSolidBrush(COLOR_MAGENTA);
            SetTextColor(hdc, RGB(0, 0, 0));
        } else {
            tabBrush = CreateSolidBrush(COLOR_BORDER);
            SetTextColor(hdc, COLOR_TEXT_MUTED);
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
 * DrawTaskKillerPage
 * Dessine le contenu de la page Task Killer
 */
static void DrawTaskKillerPage(HDC hdc, int width, int height, HFONT hFontNormal, HFONT hFontSmall) {
    TaskKillerData* data = GetTaskKillerData();
    int y = 50;  // Apres les onglets

    SelectObject(hdc, hFontNormal);

    // Titre
    SetTextColor(hdc, COLOR_CYAN);
    TextOut(hdc, 16, y, ":: TASK KILLER ::", 17);
    y += 24;

    // Separation
    HPEN sepPen = CreatePen(PS_SOLID, 1, COLOR_MAGENTA);
    SelectObject(hdc, sepPen);
    MoveToEx(hdc, 14, y, NULL);
    LineTo(hdc, width - 14, y);
    DeleteObject(sepPen);
    y += 10;

    // Positions des colonnes (alignees)
    const int COL_INDICATOR = 16;
    const int COL_PORT = 28;
    const int COL_PROCESS = 90;
    const int COL_PID = 200;

    // Constantes scrollbar
    const int SCROLLBAR_WIDTH = 8;
    const int SCROLLBAR_X = width - 12;
    const int LIST_START_Y = y + 18;  // Debut de la liste (apres en-tete)
    const int maxVisible = 8;
    const int LIST_HEIGHT = maxVisible * PROCESS_LINE_HEIGHT;

    // En-tete
    SelectObject(hdc, hFontSmall);
    SetTextColor(hdc, COLOR_TEXT_MUTED);
    TextOut(hdc, COL_PORT, y, "PORT", 4);
    TextOut(hdc, COL_PROCESS, y, "PROCESS", 7);
    TextOut(hdc, COL_PID, y, "PID", 3);
    y += 18;

    // Liste des processus
    SelectObject(hdc, hFontNormal);
    int startIdx = data->scroll_offset;
    int endIdx = startIdx + maxVisible;
    if (endIdx > data->count) endIdx = data->count;

    for (int i = startIdx; i < endIdx; i++) {
        ProcessInfo* proc = &data->processes[i];

        // Indicateur actif (point vert/gris)
        SetTextColor(hdc, proc->is_active ? COLOR_PROCESS_ACTIVE : COLOR_PROCESS_INACTIVE);
        TextOut(hdc, COL_INDICATOR, y, "*", 1);

        // Port
        char line[64];
        snprintf(line, sizeof(line), ":%d", proc->port);
        SetTextColor(hdc, COLOR_CYAN);
        TextOut(hdc, COL_PORT, y, line, (int)strlen(line));

        // Nom du processus (tronque si trop long)
        char procName[20];
        strncpy(procName, proc->name, 12);
        procName[12] = '\0';
        if (strlen(proc->name) > 12) strcat(procName, "..");
        SetTextColor(hdc, COLOR_TEXT_PRIMARY);
        TextOut(hdc, COL_PROCESS, y, procName, (int)strlen(procName));

        // PID
        snprintf(line, sizeof(line), "%lu", proc->pid);
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        TextOut(hdc, COL_PID, y, line, (int)strlen(line));

        // Bouton Kill (decale pour la scrollbar)
        RECT killRect = {width - 70, y - 2, width - 26, y + 16};
        HBRUSH killBrush = CreateSolidBrush(COLOR_KILL_BUTTON);
        FillRect(hdc, &killRect, killBrush);
        DeleteObject(killBrush);
        SetTextColor(hdc, RGB(255, 255, 255));
        SelectObject(hdc, hFontSmall);
        DrawText(hdc, "Kill", -1, &killRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, hFontNormal);

        y += PROCESS_LINE_HEIGHT;
    }

    // ===== SCROLLBAR LATERALE =====
    if (data->count > maxVisible) {
        // Fond de la scrollbar (track)
        RECT trackRect = {SCROLLBAR_X - SCROLLBAR_WIDTH/2, LIST_START_Y,
                          SCROLLBAR_X + SCROLLBAR_WIDTH/2, LIST_START_Y + LIST_HEIGHT};
        HBRUSH trackBrush = CreateSolidBrush(COLOR_BORDER);
        FillRect(hdc, &trackRect, trackBrush);
        DeleteObject(trackBrush);

        // Calculer la taille et position du thumb
        int totalItems = data->count;
        int thumbHeight = (LIST_HEIGHT * maxVisible) / totalItems;
        if (thumbHeight < 20) thumbHeight = 20;  // Hauteur minimale

        int maxScroll = totalItems - maxVisible;
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

    // Message si aucun processus
    if (data->count == 0) {
        SetTextColor(hdc, COLOR_TEXT_MUTED);
        TextOut(hdc, 16, y, "No processes with open ports", 28);
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
    char countStr[48];
    if (data->count > maxVisible) {
        snprintf(countStr, sizeof(countStr), "%d processes (%d-%d)",
                 data->count, data->scroll_offset + 1,
                 (data->scroll_offset + maxVisible > data->count) ? data->count : data->scroll_offset + maxVisible);
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
            } else {
                // Page Performance (par defaut)
                int y = 50;  // Apres les onglets

                // Logo ASCII
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
                const char* pluginOrder[] = {"CPU", "RAM", "Disk", "Process"};
                int pluginCount = 4;

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
                SetTextColor(hdc, COLOR_TEXT_MUTED);
                const char* footer = "F2 minimal | F3 hide | F4 tasks";
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
                if (i == PAGE_PRAYER) continue;  // Prayer pas encore implemente
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
                            RefreshProcessList(GetTaskKillerData());
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

                // Bouton Refresh
                if (pt.x >= 16 && pt.x <= 100 &&
                    pt.y >= height - 40 && pt.y <= height - 16) {
                    RefreshProcessList(data);
                    UpdateDisplay();
                    return 0;
                }

                // Boutons Kill pour chaque processus
                int y = 102;  // Position de la premiere ligne de processus
                int maxVisible = 8;
                int startIdx = data->scroll_offset;
                int endIdx = startIdx + maxVisible;
                if (endIdx > data->count) endIdx = data->count;

                for (int i = startIdx; i < endIdx; i++) {
                    if (pt.x >= width - 60 && pt.x <= width - 16 &&
                        pt.y >= y - 2 && pt.y <= y + 16) {
                        // Clic sur bouton Kill
                        if (KillProcessByIndex(data, i)) {
                            UpdateDisplay();
                        }
                        return 0;
                    }
                    y += PROCESS_LINE_HEIGHT;
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

        case WM_KEYDOWN:
            // Utiliser les touches configurables (v3.0)
            if (wParam == g_config.toggle_visibility_key) {
                ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
            } else if (wParam == g_config.toggle_minimal_key) {
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
