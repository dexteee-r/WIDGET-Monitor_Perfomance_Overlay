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
#include "../include/startup.h"

// Constantes
#define WINDOW_CLASS_NAME "PerformanceOverlayClass"
#define TIMER_ID 1
#define TIMER_INTERVAL 2000  // 2 secondes
#define WINDOW_WIDTH 340
#define WINDOW_HEIGHT_FULL 270
#define WINDOW_HEIGHT_MIN 180

// Palette neon
#define COLOR_BG RGB(8, 10, 18)
#define COLOR_PANEL RGB(16, 20, 30)
#define COLOR_BORDER RGB(40, 45, 60)
#define COLOR_CYAN RGB(0, 230, 255)
#define COLOR_MAGENTA RGB(255, 64, 180)
#define COLOR_TEXT_PRIMARY RGB(200, 210, 230)
#define COLOR_TEXT_MUTED RGB(120, 130, 150)

// Variables globales
HWND g_hwnd = NULL;              // Handle de la fenetre principale
Config g_config;                 // Configuration de l'application
PerformanceData g_perfData;      // Donnees de performance
BOOL g_isDragging = FALSE;       // Etat du deplacement de fenetre
POINT g_dragOffset;              // Offset lors du drag
char g_bannerTop[64] = "==[ SYSTEM OVERLAY ]==";
char g_bannerBottom[64] = ":: PERF OVERLAY v2 ::";

static int GetWindowHeightForMode(BOOL minimal) {
    return minimal ? WINDOW_HEIGHT_MIN : WINDOW_HEIGHT_FULL;
}

/*
 * BuildBarString
 * Construit une barre ASCII de largeur donnee, basee sur un pourcentage
 * Style: remplissage '=' avec un chevron '>' en tete et '.' pour le reste
 */
static void BuildBarString(char* buffer, size_t len, float percent, int width) {
    if (len == 0) {
        return;
    }
    if ((int)len <= width + 3) {
        buffer[0] = '\0';
        return;
    }

    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    int filled = (int)((percent / 100.0f) * width + 0.5f);
    if (filled > width) filled = width;
    int arrowPos = (filled > 0) ? filled - 1 : 0;

    buffer[0] = '[';
    for (int i = 0; i < width; i++) {
        if (i < filled - 1) {
            buffer[1 + i] = '=';
        } else if (i == arrowPos && filled > 0) {
            buffer[1 + i] = '>';
        } else {
            buffer[1 + i] = '.';
        }
    }
    buffer[1 + width] = ']';
    buffer[2 + width] = '\0';
}

/*
 * FormatUptime
 * Formate le temps de fonctionnement en chaine lisible
 */
static void FormatUptime(char* buffer, size_t len, DWORD seconds) {
    DWORD days = seconds / 86400;
    DWORD hours = (seconds % 86400) / 3600;
    DWORD minutes = (seconds % 3600) / 60;
    DWORD secs = seconds % 60;

    if (days > 0) {
        snprintf(buffer, len, "UPTIME %lud %02luh %02lum %02lus",
                 (unsigned long)days, (unsigned long)hours,
                 (unsigned long)minutes, (unsigned long)secs);
    } else {
        snprintf(buffer, len, "UPTIME %02luh %02lum %02lus",
                 (unsigned long)hours, (unsigned long)minutes,
                 (unsigned long)secs);
    }
}

/*
 * BuildDiskLine
 * Construit une ligne listant tous les disques detectes
 */
static void BuildDiskLine(char* buffer, size_t len, const PerformanceData* data) {
    if (len == 0) {
        return;
    }

    int written = snprintf(buffer, len, "DISKS ");
    if (written < 0 || written >= (int)len) {
        buffer[0] = '\0';
        return;
    }

    if (data->disk_count == 0) {
        snprintf(buffer, len, "DISKS n/a");
        return;
    }

    for (int i = 0; i < data->disk_count; i++) {
        int remaining = (int)len - written;
        if (remaining <= 0) {
            break;
        }

        int added = snprintf(buffer + written, remaining, "%s %.0f%%",
                             data->disk_names[i], data->disk_usages[i]);
        if (added < 0 || added >= remaining) {
            buffer[len - 1] = '\0';
            break;
        }
        written += added;

        if (i < data->disk_count - 1 && written < (int)len - 3) {
            buffer[written++] = ' ';
            buffer[written++] = '|';
            buffer[written++] = ' ';
            buffer[written] = '\0';
        }
    }
}

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
            GetPerformanceData(&g_perfData);
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
            HFONT hFontTitle = CreateFont(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
            HFONT hFontNormal = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");
            HFONT hFontSmall = CreateFont(11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN, "Consolas");

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

            // Uptime en premier plan
            char uptimeText[64];
            FormatUptime(uptimeText, sizeof(uptimeText), g_perfData.uptime_seconds);
            SetTextColor(hdc, COLOR_CYAN);
            TextOut(hdc, 16, y, uptimeText, (int)strlen(uptimeText));
            y += 12;

            // Separation
            HPEN sepPen = CreatePen(PS_SOLID, 1, COLOR_MAGENTA);
            SelectObject(hdc, sepPen);
            MoveToEx(hdc, 14, y + 6, NULL);
            LineTo(hdc, width - 14, y + 6);
            DeleteObject(sepPen);
            y += 14;

            // CPU
            char cpuBar[40];
            char cpuText[128];
            BuildBarString(cpuBar, sizeof(cpuBar), g_perfData.cpu_usage, 22);
            snprintf(cpuText, sizeof(cpuText), "CPU   %5.1f%%  %4.2f GHz  %s",
                     g_perfData.cpu_usage, g_perfData.cpu_frequency_ghz, cpuBar);
            SetTextColor(hdc, COLOR_TEXT_PRIMARY);
            TextOut(hdc, 16, y, cpuText, (int)strlen(cpuText));
            y += 18;

            // RAM
            char ramBar[40];
            char ramText[128];
            BuildBarString(ramBar, sizeof(ramBar), g_perfData.memory_usage, 22);
            snprintf(ramText, sizeof(ramText), "RAM   %4.1f/%4.1f GB  %5.1f%%  %s",
                     g_perfData.memory_used_gb, g_perfData.memory_total_gb,
                     g_perfData.memory_usage, ramBar);
            TextOut(hdc, 16, y, ramText, (int)strlen(ramText));
            y += 18;

            // Disques
            char diskLine[256];
            BuildDiskLine(diskLine, sizeof(diskLine), &g_perfData);
            SetTextColor(hdc, COLOR_CYAN);
            TextOut(hdc, 16, y, diskLine, (int)strlen(diskLine));
            y += 18;

            if (!g_config.minimal_mode) {
                // Processus + hints
                char procText[64];
                snprintf(procText, sizeof(procText), "PROC  %lu", (unsigned long)g_perfData.process_count);
                SetTextColor(hdc, COLOR_TEXT_PRIMARY);
                TextOut(hdc, 16, y, procText, (int)strlen(procText));
                y += 18;
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
                SaveConfig(&g_config);
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
            if (wParam == VK_F3) {
                ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
            } else if (wParam == VK_F2) {
                g_config.minimal_mode = !g_config.minimal_mode;
                SaveConfig(&g_config);
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, WINDOW_WIDTH, GetWindowHeightForMode(g_config.minimal_mode), SWP_NOMOVE | SWP_NOZORDER);
                UpdateDisplay();
            }
            return 0;

        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
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

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        WINDOW_CLASS_NAME,
        "Performance Overlay",
        WS_POPUP,
        g_config.x,
        g_config.y,
        WINDOW_WIDTH,
        GetWindowHeightForMode(g_config.minimal_mode),
        NULL,
        NULL,
        hInstance,
        NULL
    );

    SetLayeredWindowAttributes(hwnd, 0, 230, LWA_ALPHA);
    return hwnd;
}

/*
 * WinMain
 * Point d'entree du programme Windows
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

    InitPerformanceMonitoring();
    LoadConfig(&g_config);
    InitBanner();
    AddToStartup();

    GetPerformanceData(&g_perfData);

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

    CleanupPerformanceMonitoring();
    return 0;
}
