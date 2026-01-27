/*
 * plugin_datetime.c
 * Plugin pour afficher la date et l'heure actuelles
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

/*
 * GetDayOfWeekFR
 * Retourne le jour de la semaine en français abrégé
 */
static const char* GetDayOfWeekFR(int dayOfWeek) {
    static const char* days[] = {
        "Dim", "Lun", "Mar", "Mer", "Jeu", "Ven", "Sam"
    };
    if (dayOfWeek >= 0 && dayOfWeek <= 6) {
        return days[dayOfWeek];
    }
    return "???";
}

/*
 * GetMonthFR
 * Retourne le mois en français abrégé
 */
static const char* GetMonthFR(int month) {
    static const char* months[] = {
        "Jan", "Fev", "Mar", "Avr", "Mai", "Jun",
        "Jul", "Aou", "Sep", "Oct", "Nov", "Dec"
    };
    if (month >= 1 && month <= 12) {
        return months[month - 1];
    }
    return "???";
}

/*
 * datetime_init
 * Initialisation du plugin datetime
 */
static void datetime_init(void) {
    // Rien à initialiser
}

/*
 * datetime_update
 * Mise à jour de la date/heure
 */
static void datetime_update(MetricData* data) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    data->value = 0;  // Pas de valeur numérique
    data->line_count = 1;

    // Format: "Lun 20 Jan  14:35:22"
    snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
             "Time  %s %02d %s  %02d:%02d:%02d",
             GetDayOfWeekFR(st.wDayOfWeek),
             st.wDay,
             GetMonthFR(st.wMonth),
             st.wHour,
             st.wMinute,
             st.wSecond);

    // Couleur selon l'heure (effet visuel)
    if (st.wHour >= 6 && st.wHour < 12) {
        data->color = RGB(255, 200, 100);  // Matin - orange clair
    } else if (st.wHour >= 12 && st.wHour < 18) {
        data->color = COLOR_TEXT_PRIMARY;  // Après-midi - blanc
    } else if (st.wHour >= 18 && st.wHour < 21) {
        data->color = RGB(255, 150, 100);  // Soirée - orange
    } else {
        data->color = RGB(150, 150, 255);  // Nuit - bleu clair
    }
}

/*
 * datetime_cleanup
 * Nettoyage du plugin datetime
 */
static void datetime_cleanup(void) {
    // Rien à nettoyer
}

/*
 * datetime_is_available
 * Toujours disponible
 */
static BOOL datetime_is_available(void) {
    return TRUE;
}

// Déclaration du plugin
MetricPlugin DateTimePlugin = {
    .plugin_name = "DateTime",
    .description = "Affiche la date et l'heure actuelles",
    .init = datetime_init,
    .update = datetime_update,
    .cleanup = datetime_cleanup,
    .is_available = datetime_is_available,
    .next = NULL
};
