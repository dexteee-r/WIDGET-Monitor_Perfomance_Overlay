/*
 * constants.h
 * Constantes de configuration centralisées
 * Toutes les valeurs configurables de l'application sont ici
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <windows.h>

// ===== DIMENSIONS FENÊTRE =====
#define WINDOW_WIDTH 340
#define WINDOW_HEIGHT_FULL 270
#define WINDOW_HEIGHT_MIN 180

// ===== TIMERS ET PERFORMANCE =====
#define TIMER_ID 1
#define TIMER_INTERVAL 2000  // Millisecondes (2 secondes)

// ===== PALETTE COULEURS NEON =====
// Fond et panneaux
#define COLOR_BG RGB(8, 10, 18)
#define COLOR_PANEL RGB(16, 20, 30)
#define COLOR_BORDER RGB(40, 45, 60)

// Couleurs d'accent
#define COLOR_CYAN RGB(0, 230, 255)
#define COLOR_MAGENTA RGB(255, 64, 180)

// Texte
#define COLOR_TEXT_PRIMARY RGB(200, 210, 230)
#define COLOR_TEXT_MUTED RGB(120, 130, 150)

// ===== TRANSPARENCE =====
#define WINDOW_OPACITY 230  // 0-255 (230 = ~90% opaque)

// ===== LIMITES =====
#define MAX_DISKS 8  // Nombre maximum de disques à surveiller

// ===== INTERFACE =====
#define WINDOW_CLASS_NAME "PerformanceOverlayClass"

// Polices
#define FONT_TITLE_NAME "Consolas"
#define FONT_TITLE_SIZE 16
#define FONT_NORMAL_NAME "Consolas"
#define FONT_NORMAL_SIZE 12
#define FONT_SMALL_NAME "Consolas"
#define FONT_SMALL_SIZE 11

// Largeurs des barres de progression
#define PROGRESS_BAR_WIDTH 22

// ===== FICHIERS =====
#define CONFIG_FILE "config.txt"
#define CONFIG_FILE_INI "config.ini"

#endif // CONSTANTS_H
