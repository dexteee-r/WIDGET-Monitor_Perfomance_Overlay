/*
 * main.c
 * Programme principal - Overlay de performance système
 *
 * Ce programme affiche en temps réel les performances du système
 * dans une fenêtre transparente toujours visible
 */

#include <windows.h>
#include <stdio.h>
#include "../include/performance.h"
#include "../include/config.h"
#include "../include/startup.h"

// Constantes
#define WINDOW_CLASS_NAME "PerformanceOverlayClass"
#define TIMER_ID 1
#define TIMER_INTERVAL 2000  // 2 secondes

// Variables globales
HWND g_hwnd = NULL;              // Handle de la fenêtre principale
Config g_config;                 // Configuration de l'application
PerformanceData g_perfData;      // Données de performance
BOOL g_isDragging = FALSE;       // État du déplacement de fenêtre
POINT g_dragOffset;              // Offset lors du drag

/*
 * UpdateDisplay
 * Met à jour l'affichage des informations de performance
 */
void UpdateDisplay() {
    // Forcer le redessin de la fenêtre
    InvalidateRect(g_hwnd, NULL, TRUE);
}

/*
 * WindowProc
 * Fonction de traitement des messages de la fenêtre
 * C'est le "cerveau" de notre interface graphique
 */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // Message reçu lors de la création de la fenêtre
            // Démarrer le timer pour la mise à jour automatique
            SetTimer(hwnd, TIMER_ID, TIMER_INTERVAL, NULL);
            return 0;

        case WM_TIMER:
            // Message reçu toutes les 2 secondes
            // Récupérer les nouvelles données de performance
            GetPerformanceData(&g_perfData);
            UpdateDisplay();
            return 0;

        case WM_PAINT: {
            // Message reçu quand il faut dessiner la fenêtre
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Obtenir les dimensions de la fenêtre
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right;
            int height = clientRect.bottom;

            // === DESSINER LE FOND ===
            HBRUSH bgBrush = CreateSolidBrush(RGB(30, 30, 35));
            FillRect(hdc, &clientRect, bgBrush);
            DeleteObject(bgBrush);

            // Bordure supérieure (accent bleu)
            HBRUSH accentBrush = CreateSolidBrush(RGB(0, 120, 215));
            RECT accentRect = {0, 0, width, 4};
            FillRect(hdc, &accentRect, accentBrush);
            DeleteObject(accentBrush);

            // === CRÉER LES POLICES ===
            HFONT hFontTitle = CreateFont(14, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            HFONT hFontNormal = CreateFont(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
            HFONT hFontSmall = CreateFont(10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");

            SetBkMode(hdc, TRANSPARENT);

            // === TITRE ===
            SelectObject(hdc, hFontTitle);
            SetTextColor(hdc, RGB(200, 200, 200));
            RECT titleRect = {10, 10, width - 30, 30};
            DrawText(hdc, "Performance Monitor", -1, &titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            // === BOUTON FERMETURE ===
            HBRUSH closeBrush = CreateSolidBrush(RGB(255, 50, 50));
            RECT closeRect = {width - 25, 8, width - 5, 28};
            FillRect(hdc, &closeRect, closeBrush);
            DeleteObject(closeBrush);
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, RGB(255, 255, 255));
            DrawText(hdc, "X", -1, &closeRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            int yPos = 40;
            SelectObject(hdc, hFontNormal);

            if (!g_config.minimal_mode) {
                // === MODE COMPLET ===
                SetTextColor(hdc, RGB(180, 180, 180));

                // CPU
                char cpuText[64];
                sprintf(cpuText, "CPU: %.1f%% (%.2f GHz)", g_perfData.cpu_usage, g_perfData.cpu_frequency_ghz);
                TextOut(hdc, 10, yPos, cpuText, strlen(cpuText));
                yPos += 18;

                // Barre CPU
                HBRUSH barBg1 = CreateSolidBrush(RGB(50, 50, 55));
                RECT barRect1 = {10, yPos, width - 10, yPos + 12};
                FillRect(hdc, &barRect1, barBg1);
                DeleteObject(barBg1);
                int barW1 = (int)((width - 24) * (g_perfData.cpu_usage / 100.0f));
                if (barW1 > 0) {
                    COLORREF c1 = (g_perfData.cpu_usage < 60) ? RGB(0, 200, 100) : (g_perfData.cpu_usage < 80) ? RGB(255, 180, 0) : RGB(255, 70, 70);
                    HBRUSH bar1 = CreateSolidBrush(c1);
                    RECT filled1 = {12, yPos + 2, 12 + barW1, yPos + 10};
                    FillRect(hdc, &filled1, bar1);
                    DeleteObject(bar1);
                }
                yPos += 22;

                // RAM
                char ramText[64];
                sprintf(ramText, "RAM: %.1f / %.1f GB", g_perfData.memory_used_gb, g_perfData.memory_total_gb);
                TextOut(hdc, 10, yPos, ramText, strlen(ramText));
                yPos += 18;

                // Barre RAM
                HBRUSH barBg2 = CreateSolidBrush(RGB(50, 50, 55));
                RECT barRect2 = {10, yPos, width - 10, yPos + 12};
                FillRect(hdc, &barRect2, barBg2);
                DeleteObject(barBg2);
                int barW2 = (int)((width - 24) * (g_perfData.memory_usage / 100.0f));
                if (barW2 > 0) {
                    COLORREF c2 = (g_perfData.memory_usage < 60) ? RGB(0, 200, 100) : (g_perfData.memory_usage < 80) ? RGB(255, 180, 0) : RGB(255, 70, 70);
                    HBRUSH bar2 = CreateSolidBrush(c2);
                    RECT filled2 = {12, yPos + 2, 12 + barW2, yPos + 10};
                    FillRect(hdc, &filled2, bar2);
                    DeleteObject(bar2);
                }
                yPos += 22;

                // Disque
                char diskText[64];
                sprintf(diskText, "Disk %s: %.1f%%", g_perfData.disk_name, g_perfData.disk_usage);
                TextOut(hdc, 10, yPos, diskText, strlen(diskText));
                yPos += 18;

                // Barre Disque
                HBRUSH barBg3 = CreateSolidBrush(RGB(50, 50, 55));
                RECT barRect3 = {10, yPos, width - 10, yPos + 12};
                FillRect(hdc, &barRect3, barBg3);
                DeleteObject(barBg3);
                int barW3 = (int)((width - 24) * (g_perfData.disk_usage / 100.0f));
                if (barW3 > 0) {
                    COLORREF c3 = (g_perfData.disk_usage < 60) ? RGB(0, 200, 100) : (g_perfData.disk_usage < 80) ? RGB(255, 180, 0) : RGB(255, 70, 70);
                    HBRUSH bar3 = CreateSolidBrush(c3);
                    RECT filled3 = {12, yPos + 2, 12 + barW3, yPos + 10};
                    FillRect(hdc, &filled3, bar3);
                    DeleteObject(bar3);
                }
                yPos += 22;

                // Ligne de séparation
                HPEN sepPen = CreatePen(PS_SOLID, 1, RGB(60, 60, 65));
                SelectObject(hdc, sepPen);
                MoveToEx(hdc, 10, yPos, NULL);
                LineTo(hdc, width - 10, yPos);
                DeleteObject(sepPen);
                yPos += 10;

                // Infos supplémentaires
                SelectObject(hdc, hFontSmall);
                SetTextColor(hdc, RGB(150, 150, 150));

                char uptimeText[64];
                DWORD hours = g_perfData.uptime_seconds / 3600;
                DWORD minutes = (g_perfData.uptime_seconds % 3600) / 60;
                sprintf(uptimeText, "Uptime: %dh %dm", hours, minutes);
                TextOut(hdc, 10, yPos, uptimeText, strlen(uptimeText));
                yPos += 18;

                char processText[64];
                sprintf(processText, "Processes: %d", g_perfData.process_count);
                TextOut(hdc, 10, yPos, processText, strlen(processText));
                yPos += 18;

                SetTextColor(hdc, RGB(100, 100, 100));
                TextOut(hdc, 10, yPos, "F3: Hide | F2: Minimal", 22);

            } else {
                // === MODE MINIMAL ===
                SetTextColor(hdc, RGB(180, 180, 180));

                char cpuText[64];
                sprintf(cpuText, "CPU: %.1f%%", g_perfData.cpu_usage);
                TextOut(hdc, 10, yPos, cpuText, strlen(cpuText));
                yPos += 18;

                HBRUSH barBg1 = CreateSolidBrush(RGB(50, 50, 55));
                RECT barRect1 = {10, yPos, width - 10, yPos + 10};
                FillRect(hdc, &barRect1, barBg1);
                DeleteObject(barBg1);
                int barW1 = (int)((width - 24) * (g_perfData.cpu_usage / 100.0f));
                if (barW1 > 0) {
                    HBRUSH bar1 = CreateSolidBrush(RGB(0, 200, 100));
                    RECT filled1 = {12, yPos + 2, 12 + barW1, yPos + 8};
                    FillRect(hdc, &filled1, bar1);
                    DeleteObject(bar1);
                }
                yPos += 20;

                char ramText[64];
                sprintf(ramText, "RAM: %.1f GB", g_perfData.memory_used_gb);
                TextOut(hdc, 10, yPos, ramText, strlen(ramText));
                yPos += 18;

                HBRUSH barBg2 = CreateSolidBrush(RGB(50, 50, 55));
                RECT barRect2 = {10, yPos, width - 10, yPos + 10};
                FillRect(hdc, &barRect2, barBg2);
                DeleteObject(barBg2);
                int barW2 = (int)((width - 24) * (g_perfData.memory_usage / 100.0f));
                if (barW2 > 0) {
                    HBRUSH bar2 = CreateSolidBrush(RGB(0, 200, 100));
                    RECT filled2 = {12, yPos + 2, 12 + barW2, yPos + 8};
                    FillRect(hdc, &filled2, bar2);
                    DeleteObject(bar2);
                }
            }

            DeleteObject(hFontTitle);
            DeleteObject(hFontNormal);
            DeleteObject(hFontSmall);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            // Clic gauche - commencer le déplacement
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};

            // Vérifier si on clique sur le bouton X
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int width = clientRect.right;
            if (pt.x >= width - 25 && pt.x <= width - 5 && pt.y >= 8 && pt.y <= 28) {
                PostQuitMessage(0);
                return 0;
            }

            // Sinon, commencer le drag
            g_isDragging = TRUE;
            g_dragOffset.x = pt.x;
            g_dragOffset.y = pt.y;
            SetCapture(hwnd);
            return 0;
        }

        case WM_LBUTTONUP:
            // Relâcher le bouton - arrêter le déplacement
            if (g_isDragging) {
                g_isDragging = FALSE;
                ReleaseCapture();

                // Sauvegarder la nouvelle position
                RECT rect;
                GetWindowRect(hwnd, &rect);
                g_config.x = rect.left;
                g_config.y = rect.top;
                SaveConfig(&g_config);
            }
            return 0;

        case WM_MOUSEMOVE:
            // Déplacer la fenêtre si on est en train de dragger
            if (g_isDragging) {
                POINT pt;
                GetCursorPos(&pt);
                SetWindowPos(
                    hwnd,
                    HWND_TOPMOST,
                    pt.x - g_dragOffset.x,
                    pt.y - g_dragOffset.y,
                    0, 0,
                    SWP_NOSIZE | SWP_NOZORDER
                );
            }
            return 0;

        case WM_KEYDOWN:
            // Touche pressée
            if (wParam == VK_F3) {
                // F3 - Afficher/Masquer
                ShowWindow(hwnd, IsWindowVisible(hwnd) ? SW_HIDE : SW_SHOW);
            } else if (wParam == VK_F2) {
                // F2 - Basculer mode minimaliste
                g_config.minimal_mode = !g_config.minimal_mode;
                SaveConfig(&g_config);
                UpdateDisplay();
            }
            return 0;

        case WM_DESTROY:
            // Message reçu lors de la fermeture
            KillTimer(hwnd, TIMER_ID);
            PostQuitMessage(0);
            return 0;
    }

    // Message non géré - laisser Windows le traiter
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/*
 * CreateOverlayWindow
 * Crée la fenêtre de l'overlay
 */
HWND CreateOverlayWindow(HINSTANCE hInstance) {
    // 1. Enregistrer la classe de fenêtre
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;           // Fonction de traitement
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));  // Fond noir

    RegisterClass(&wc);

    // 2. Créer la fenêtre (dimensions augmentées pour le nouveau design)
    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,  // Styles étendus
        WINDOW_CLASS_NAME,
        "Performance Overlay",
        WS_POPUP,                          // Style popup (sans bordure)
        g_config.x,                        // Position X
        g_config.y,                        // Position Y
        280,                               // Largeur (augmentée)
        g_config.minimal_mode ? 120 : 240, // Hauteur (augmentée)
        NULL,
        NULL,
        hInstance,
        NULL
    );

    // 3. Rendre la fenêtre semi-transparente
    SetLayeredWindowAttributes(hwnd, 0, 250, LWA_ALPHA);

    return hwnd;
}

/*
 * DrawCloseButton
 * Dessine le bouton de fermeture X
 */
void DrawCloseButton(HWND hwnd) {
    HDC hdc = GetDC(hwnd);

    // Dessiner un rectangle rouge
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
    RECT rect = {5, 5, 25, 25};
    FillRect(hdc, &rect, hBrush);

    // Dessiner le X blanc
    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    DrawText(hdc, "X", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    DeleteObject(hBrush);
    ReleaseDC(hwnd, hdc);
}

/*
 * WinMain
 * Point d'entrée du programme Windows
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {

    // 1. Initialiser le monitoring
    InitPerformanceMonitoring();

    // 2. Charger la configuration
    LoadConfig(&g_config);

    // 3. Ajouter au démarrage Windows
    AddToStartup();

    // 4. Créer la fenêtre
    g_hwnd = CreateOverlayWindow(hInstance);
    if (g_hwnd == NULL) {
        MessageBox(NULL, "Erreur de création de la fenêtre!", "Erreur", MB_ICONERROR);
        return 1;
    }

    // 5. Afficher la fenêtre
    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    // 6. Dessiner le bouton de fermeture
    DrawCloseButton(g_hwnd);

    // 7. Obtenir les premières données
    GetPerformanceData(&g_perfData);

    // 8. Boucle de messages principale
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 9. Nettoyer avant de quitter
    CleanupPerformanceMonitoring();

    return 0;
}
