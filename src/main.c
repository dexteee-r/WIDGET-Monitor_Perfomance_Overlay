/*
 * main.c
 * Programme principal - Overlay de performance systeme
 * Version 3.1 - Code refactorise
 */

#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <string.h>
#include "../include/performance.h"
#include "../include/config_parser.h"
#include "../include/startup.h"
#include "../include/constants.h"
#include "../include/metric_plugin.h"
#include "../include/all_plugins.h"
#include "../include/taskkiller.h"
#include "../include/theme.h"
#include "../include/tray.h"
#include "../include/ui_draw.h"

// Variables globales
HWND g_hwnd = NULL;
ConfigINI g_config;
BOOL g_isDragging = FALSE;
POINT g_dragOffset;
char g_bannerTop[64] = "==[ SYSTEM OVERLAY v3 ]==";
char g_bannerBottom[64] = ":: PLUGIN SYSTEM ::";

// Systeme de pages
int g_currentPage = PAGE_PERF;
const char* g_pageNames[] = {"Perf", "Tasks", "Settings"};

static int GetWindowHeightForMode(BOOL minimal) {
    if (g_currentPage == PAGE_TASKS) {
        return WINDOW_HEIGHT_TASKS;
    }
    if (g_currentPage == PAGE_SETTINGS) {
        return WINDOW_HEIGHT_SETTINGS;
    }
    return minimal ? WINDOW_HEIGHT_MIN : WINDOW_HEIGHT_FULL;
}

/*
 * InitBanner
 * Prepare les lignes d'entete
 */
static void InitBanner(void) {
    char user[64] = {0};
    DWORD sz = (DWORD)sizeof(user);
    if (GetUserNameA(user, &sz) && sz > 1) {
        snprintf(g_bannerTop, sizeof(g_bannerTop), "==[ %s OVERLAY ]==", "SYSTEM");
        snprintf(g_bannerBottom, sizeof(g_bannerBottom), ":: %s @ overlay ::", user);
    } else {
        snprintf(g_bannerTop, sizeof(g_bannerTop), "==[ SYSTEM OVERLAY ]==");
        snprintf(g_bannerBottom, sizeof(g_bannerBottom), ":: PERF OVERLAY v3 ::");
    }
}

/*
 * UpdateDisplay
 * Met a jour l'affichage
 */
void UpdateDisplay(void) {
    InvalidateRect(g_hwnd, NULL, TRUE);
}

/*
 * WindowProc
 * Fonction de traitement des messages de la fenetre
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
            HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
            CreateTrayIcon(hwnd, hInst);
            return 0;
        }

        case WM_TRAYICON:
            if (lParam == WM_RBUTTONUP) {
                ShowTrayMenu(hwnd);
            } else if (lParam == WM_LBUTTONDBLCLK) {
                ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
                if (IsWindowVisible(hwnd)) {
                    SetForegroundWindow(hwnd);
                }
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_TRAY_SHOW:
                    ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
                    if (IsWindowVisible(hwnd)) {
                        SetForegroundWindow(hwnd);
                    }
                    break;
                case ID_TRAY_SETTINGS:
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

            // Bouton X
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

            // Onglets
            DrawTabs(hdc, width, hFontSmall);

            // Bouton X texte
            SetTextColor(hdc, RGB(0, 0, 0));
            SelectObject(hdc, hFontSmall);
            TextOut(hdc, closeRect.left + 5, closeRect.top + 2, "X", 1);

            // Contenu selon la page
            if (g_currentPage == PAGE_TASKS) {
                DrawTaskKillerPage(hdc, width, height, hFontNormal, hFontSmall);
            } else if (g_currentPage == PAGE_SETTINGS) {
                DrawSettingsPage(hdc, width, height, hFontNormal, hFontSmall);
            } else {
                // Page Performance
                int y = 50;

                // Logo ASCII
                SelectObject(hdc, hFontTitle);
                SetTextColor(hdc, g_colorAccent);
                TextOut(hdc, 16, y, g_bannerTop, (int)strlen(g_bannerTop));
                y += 18;
                SelectObject(hdc, hFontNormal);
                SetTextColor(hdc, g_colorAccent2);
                TextOut(hdc, 16, y, g_bannerBottom, (int)strlen(g_bannerBottom));
                y += 16;

                // Uptime
                MetricData* uptime = GetMetricByName("Uptime");
                if (uptime && uptime->enabled && g_config.show_uptime) {
                    SetTextColor(hdc, uptime->color);
                    TextOut(hdc, 16, y, uptime->display_lines[0], (int)strlen(uptime->display_lines[0]));
                    y += 12;
                }

                // Prayer
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

                // Metriques
                const char* pluginOrder[] = {"CPU", "RAM", "GPU", "Disk", "Process", "Volume", "DateTime"};
                int pluginCount = 7;

                for (int i = 0; i < pluginCount; i++) {
                    MetricData* metric = GetMetricByName(pluginOrder[i]);
                    if (metric && metric->enabled) {
                        if (g_config.minimal_mode && strcmp(pluginOrder[i], "Disk") == 0) continue;
                        if (g_config.minimal_mode && strcmp(pluginOrder[i], "Process") == 0) continue;
                        if (!g_config.show_processes && strcmp(pluginOrder[i], "Process") == 0) continue;

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
                TextOut(hdc, 16, height - 26, "F2 minimal | F4 tasks", 21);
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

            // Bouton X
            if (pt.x >= width - 28 && pt.x <= width - 6 && pt.y >= 10 && pt.y <= 28) {
                PostQuitMessage(0);
                return 0;
            }

            // Onglets
            int tabX = 16;
            int tabY = 12;
            for (int i = 0; i < PAGE_COUNT; i++) {
                if (pt.x >= tabX && pt.x <= tabX + TAB_WIDTH &&
                    pt.y >= tabY && pt.y <= tabY + TAB_HEIGHT) {
                    if (g_currentPage != i) {
                        g_currentPage = i;
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, WINDOW_WIDTH,
                                    GetWindowHeightForMode(g_config.minimal_mode),
                                    SWP_NOMOVE | SWP_NOZORDER);
                        if (g_currentPage == PAGE_TASKS) {
                            RefreshCurrentView(GetTaskKillerData());
                        }
                        UpdateDisplay();
                    }
                    return 0;
                }
                tabX += TAB_WIDTH + TAB_MARGIN;
            }

            // Page Task Killer
            if (g_currentPage == PAGE_TASKS) {
                TaskKillerData* data = GetTaskKillerData();
                const int TOGGLE_Y = 50;
                const int TOGGLE_HEIGHT = 18;
                const int SEARCH_Y = 72;
                const int SEARCH_HEIGHT = 20;
                const int SCROLLBAR_WIDTH = 8;
                const int SCROLLBAR_X = width - 12;
                const int LIST_START_Y = 118;
                const int maxVisible = 6;
                const int LIST_HEIGHT = maxVisible * PROCESS_LINE_HEIGHT;
                int filteredCount = GetFilteredProcessCount(data);

                // Toggle All
                if (pt.x >= 16 && pt.x <= 56 && pt.y >= TOGGLE_Y && pt.y <= TOGGLE_Y + TOGGLE_HEIGHT) {
                    if (data->view_mode != VIEW_MODE_ALL) {
                        data->view_mode = VIEW_MODE_ALL;
                        data->scroll_offset = 0;
                        RefreshCurrentView(data);
                        UpdateDisplay();
                    }
                    return 0;
                }

                // Toggle Ports
                if (pt.x >= 58 && pt.x <= 108 && pt.y >= TOGGLE_Y && pt.y <= TOGGLE_Y + TOGGLE_HEIGHT) {
                    if (data->view_mode != VIEW_MODE_PORTS) {
                        data->view_mode = VIEW_MODE_PORTS;
                        data->scroll_offset = 0;
                        RefreshCurrentView(data);
                        UpdateDisplay();
                    }
                    return 0;
                }

                // Champ recherche
                if (pt.x >= 16 && pt.x <= width - 16 && pt.y >= SEARCH_Y && pt.y <= SEARCH_Y + SEARCH_HEIGHT) {
                    if (pt.x >= width - 35 && data->filter_text[0] != '\0') {
                        data->filter_text[0] = '\0';
                        data->scroll_offset = 0;
                    }
                    data->filter_active = TRUE;
                    UpdateDisplay();
                    return 0;
                } else if (data->filter_active) {
                    data->filter_active = FALSE;
                    UpdateDisplay();
                }

                // Bouton Refresh
                if (pt.x >= 16 && pt.x <= 100 && pt.y >= height - 40 && pt.y <= height - 16) {
                    RefreshCurrentView(data);
                    data->scroll_offset = 0;
                    UpdateDisplay();
                    return 0;
                }

                // Scrollbar
                if (filteredCount > maxVisible &&
                    pt.x >= SCROLLBAR_X - SCROLLBAR_WIDTH && pt.x <= SCROLLBAR_X + SCROLLBAR_WIDTH &&
                    pt.y >= LIST_START_Y && pt.y <= LIST_START_Y + LIST_HEIGHT) {
                    int clickPos = pt.y - LIST_START_Y;
                    int maxScroll = filteredCount - maxVisible;
                    int newOffset = (clickPos * maxScroll) / LIST_HEIGHT;
                    if (newOffset < 0) newOffset = 0;
                    if (newOffset > maxScroll) newOffset = maxScroll;
                    data->scroll_offset = newOffset;
                    UpdateDisplay();
                    return 0;
                }

                // Boutons Kill
                int y = LIST_START_Y;
                int startIdx = data->scroll_offset;
                int endIdx = startIdx + maxVisible;
                if (endIdx > filteredCount) endIdx = filteredCount;

                for (int visibleIdx = startIdx; visibleIdx < endIdx; visibleIdx++) {
                    if (pt.x >= width - 70 && pt.x <= width - 26 && pt.y >= y - 2 && pt.y <= y + 16) {
                        int realIdx = GetRealIndexFromVisibleIndex(data, visibleIdx);
                        if (realIdx >= 0 && KillProcessByIndex(data, realIdx)) {
                            UpdateDisplay();
                        }
                        return 0;
                    }
                    y += PROCESS_LINE_HEIGHT;
                }
            }

            // Page Settings
            if (g_currentPage == PAGE_SETTINGS) {
                int y = 80;
                for (int i = 0; i < g_themeCount; i++) {
                    if (pt.x >= 16 && pt.x <= width - 16 && pt.y >= y && pt.y <= y + 28) {
                        ApplyTheme(i);
                        g_config.theme_index = i;
                        SaveConfigINI(&g_config, CONFIG_FILE_INI);
                        UpdateDisplay();
                        return 0;
                    }
                    y += 32;
                }
            }

            // Drag fenetre
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
            if (g_currentPage == PAGE_TASKS) {
                TaskKillerData* data = GetTaskKillerData();
                const int maxVisible = 6;
                int filteredCount = GetFilteredProcessCount(data);

                if (filteredCount > maxVisible) {
                    int delta = GET_WHEEL_DELTA_WPARAM(wParam);
                    data->scroll_offset -= delta / WHEEL_DELTA;
                    int maxScroll = filteredCount - maxVisible;
                    if (data->scroll_offset < 0) data->scroll_offset = 0;
                    if (data->scroll_offset > maxScroll) data->scroll_offset = maxScroll;
                    UpdateDisplay();
                }
            }
            return 0;
        }

        case WM_CHAR: {
            if (g_currentPage == PAGE_TASKS) {
                TaskKillerData* data = GetTaskKillerData();
                if (data->filter_active) {
                    char c = (char)wParam;
                    int len = (int)strlen(data->filter_text);

                    if (c == '\b' && len > 0) {
                        data->filter_text[len - 1] = '\0';
                        data->scroll_offset = 0;
                        UpdateDisplay();
                    } else if (c == 27) {
                        data->filter_active = FALSE;
                        UpdateDisplay();
                    } else if (c >= 32 && c < 127 && len < 30) {
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
            if (wParam == g_config.toggle_minimal_key && g_currentPage == PAGE_PERF) {
                g_config.minimal_mode = !g_config.minimal_mode;
                SaveConfigINI(&g_config, CONFIG_FILE_INI);
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, WINDOW_WIDTH,
                            GetWindowHeightForMode(g_config.minimal_mode), SWP_NOMOVE | SWP_NOZORDER);
                UpdateDisplay();
            } else if (wParam == VK_F4) {
                g_currentPage = (g_currentPage == PAGE_PERF) ? PAGE_TASKS : PAGE_PERF;
                if (g_currentPage == PAGE_TASKS) {
                    RefreshProcessList(GetTaskKillerData());
                }
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, WINDOW_WIDTH,
                            GetWindowHeightForMode(g_config.minimal_mode), SWP_NOMOVE | SWP_NOZORDER);
                UpdateDisplay();
            } else if (wParam == g_config.reload_config_key) {
                LoadConfigINI(&g_config, CONFIG_FILE_INI);
                UpdateDisplay();
            }
            return 0;

        case WM_DESTROY:
            RemoveTrayIcon();
            KillTimer(hwnd, TIMER_ID);
            CleanupTaskKiller();
            CleanupPluginSystem();
            CleanupPerformanceMonitoring();
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
 * CreateOverlayWindow
 */
HWND CreateOverlayWindow(HINSTANCE hInstance) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));

    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int posX = g_config.x;
    int posY = g_config.y;

    if (posX < 0 || posX > screenWidth - 100 || posY < 0 || posY > screenHeight - 100) {
        posX = (screenWidth - WINDOW_WIDTH) / 2;
        posY = (screenHeight - GetWindowHeightForMode(g_config.minimal_mode)) / 2;
    }

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        WINDOW_CLASS_NAME,
        "Performance Overlay",
        WS_POPUP,
        posX, posY,
        WINDOW_WIDTH,
        GetWindowHeightForMode(g_config.minimal_mode),
        NULL, NULL, hInstance, NULL
    );

    SetLayeredWindowAttributes(hwnd, 0, WINDOW_OPACITY, LWA_ALPHA);
    return hwnd;
}

/*
 * WinMain
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

    InitPerformanceMonitoring();
    LoadConfigINI(&g_config, CONFIG_FILE_INI);
    ApplyTheme(g_config.theme_index);

    // Plugins
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

    SetPrayerConfig(g_config.prayer_city, g_config.prayer_country,
                    g_config.prayer_method, g_config.prayer_use_api,
                    g_config.prayer_enabled,
                    g_config.prayer_fajr, g_config.prayer_dhuhr,
                    g_config.prayer_asr, g_config.prayer_maghrib,
                    g_config.prayer_isha);

    EnablePlugin("CPU", g_config.cpu_enabled);
    EnablePlugin("RAM", g_config.ram_enabled);
    EnablePlugin("Disk", g_config.disk_enabled);
    EnablePlugin("Uptime", g_config.uptime_enabled);
    EnablePlugin("Process", g_config.process_enabled);
    EnablePlugin("Prayer", g_config.prayer_enabled);
    EnablePlugin("Network", TRUE);
    EnablePlugin("DateTime", TRUE);
    EnablePlugin("Volume", TRUE);
    EnablePlugin("GPU", TRUE);

    InitTaskKiller();
    InitBanner();
    AddToStartup();
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

    return 0;
}
