/*
 * plugin_prayer.c
 * Plugin pour afficher la prochaine priere
 * Utilise l'API Aladhan pour les horaires automatiques
 */

#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

#pragma comment(lib, "wininet.lib")

// Structure pour stocker les horaires de priere
typedef struct {
    const char* name;
    int hour;
    int minute;
} PrayerTime;

// Configuration
static char g_city[64] = "Paris";
static char g_country[64] = "France";
static int g_method = 2;  // ISNA par defaut
static BOOL g_useApi = TRUE;
static BOOL g_prayerEnabled = TRUE;

// Horaires de priere
static PrayerTime g_prayers[5] = {
    {"Fajr", 6, 0},
    {"Dhuhr", 13, 0},
    {"Asr", 16, 0},
    {"Maghrib", 19, 0},
    {"Isha", 21, 0}
};

// Cache pour eviter les appels API trop frequents
static int g_lastFetchDay = -1;
static BOOL g_apiFailed = FALSE;

/*
 * ParseTimeString
 * Parse une chaine "HH:MM" en heures et minutes
 */
static void ParseTimeString(const char* timeStr, int* hour, int* minute) {
    if (!timeStr || !hour || !minute) return;
    *hour = 0;
    *minute = 0;
    int h = 0, m = 0;
    if (sscanf(timeStr, "%d:%d", &h, &m) == 2) {
        *hour = h;
        *minute = m;
    }
}

/*
 * ExtractJsonValue
 * Extrait une valeur simple d'un JSON (ex: "Fajr": "05:30")
 */
static BOOL ExtractJsonValue(const char* json, const char* key, char* value, size_t valueSize) {
    if (!json || !key || !value || valueSize == 0) return FALSE;

    // Chercher "key":
    char searchKey[128];
    snprintf(searchKey, sizeof(searchKey), "\"%s\"", key);

    const char* keyPos = strstr(json, searchKey);
    if (!keyPos) return FALSE;

    // Trouver le : apres la cle
    const char* colonPos = strchr(keyPos + strlen(searchKey), ':');
    if (!colonPos) return FALSE;

    // Trouver le premier "
    const char* startQuote = strchr(colonPos, '"');
    if (!startQuote) return FALSE;
    startQuote++;

    // Trouver le " fermant
    const char* endQuote = strchr(startQuote, '"');
    if (!endQuote) return FALSE;

    // Copier la valeur
    size_t len = endQuote - startQuote;
    if (len >= valueSize) len = valueSize - 1;
    strncpy(value, startQuote, len);
    value[len] = '\0';

    return TRUE;
}

/*
 * FetchPrayerTimesFromAPI
 * Recupere les horaires depuis l'API Aladhan
 */
static BOOL FetchPrayerTimesFromAPI(void) {
    BOOL success = FALSE;
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;

    // Construire l'URL
    char url[512];
    snprintf(url, sizeof(url),
             "/v1/timingsByCity?city=%s&country=%s&method=%d",
             g_city, g_country, g_method);

    // Ouvrir la connexion Internet
    hInternet = InternetOpenA("PrayerWidget/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInternet) {
        goto cleanup;
    }

    // Se connecter au serveur
    hConnect = InternetOpenUrlA(hInternet,
        "https://api.aladhan.com/v1/timingsByCity",
        NULL, 0, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);

    // Construire la requete complete
    char fullUrl[512];
    snprintf(fullUrl, sizeof(fullUrl), "https://api.aladhan.com%s", url);

    if (hConnect) {
        InternetCloseHandle(hConnect);
        hConnect = NULL;
    }

    hConnect = InternetOpenUrlA(hInternet, fullUrl, NULL, 0,
                                INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        goto cleanup;
    }

    // Lire la reponse
    char buffer[8192] = {0};
    char* response = buffer;
    DWORD bytesRead = 0;
    DWORD totalRead = 0;

    while (InternetReadFile(hConnect, response + totalRead,
                            sizeof(buffer) - totalRead - 1, &bytesRead) && bytesRead > 0) {
        totalRead += bytesRead;
        if (totalRead >= sizeof(buffer) - 1) break;
    }
    buffer[totalRead] = '\0';

    // Verifier si la reponse contient des donnees valides
    if (totalRead == 0 || strstr(buffer, "\"timings\"") == NULL) {
        goto cleanup;
    }

    // Extraire les horaires
    char timeValue[16];

    if (ExtractJsonValue(buffer, "Fajr", timeValue, sizeof(timeValue))) {
        ParseTimeString(timeValue, &g_prayers[0].hour, &g_prayers[0].minute);
    }
    if (ExtractJsonValue(buffer, "Dhuhr", timeValue, sizeof(timeValue))) {
        ParseTimeString(timeValue, &g_prayers[1].hour, &g_prayers[1].minute);
    }
    if (ExtractJsonValue(buffer, "Asr", timeValue, sizeof(timeValue))) {
        ParseTimeString(timeValue, &g_prayers[2].hour, &g_prayers[2].minute);
    }
    if (ExtractJsonValue(buffer, "Maghrib", timeValue, sizeof(timeValue))) {
        ParseTimeString(timeValue, &g_prayers[3].hour, &g_prayers[3].minute);
    }
    if (ExtractJsonValue(buffer, "Isha", timeValue, sizeof(timeValue))) {
        ParseTimeString(timeValue, &g_prayers[4].hour, &g_prayers[4].minute);
    }

    success = TRUE;

cleanup:
    if (hConnect) InternetCloseHandle(hConnect);
    if (hInternet) InternetCloseHandle(hInternet);

    return success;
}

/*
 * RefreshPrayerTimesIfNeeded
 * Rafraichit les horaires si necessaire (nouveau jour)
 */
static void RefreshPrayerTimesIfNeeded(void) {
    if (!g_useApi) return;

    SYSTEMTIME st;
    GetLocalTime(&st);

    // Rafraichir si nouveau jour
    if (st.wDay != g_lastFetchDay) {
        if (FetchPrayerTimesFromAPI()) {
            g_lastFetchDay = st.wDay;
            g_apiFailed = FALSE;
        } else {
            g_apiFailed = TRUE;
        }
    }
}

/*
 * SetPrayerConfig
 * Configure le plugin avec tous les parametres
 */
void SetPrayerConfig(const char* city, const char* country, int method,
                     BOOL useApi, BOOL enabled,
                     const char* fajr, const char* dhuhr, const char* asr,
                     const char* maghrib, const char* isha) {
    // Sauvegarder la configuration
    if (city) {
        strncpy(g_city, city, sizeof(g_city) - 1);
        g_city[sizeof(g_city) - 1] = '\0';
    }
    if (country) {
        strncpy(g_country, country, sizeof(g_country) - 1);
        g_country[sizeof(g_country) - 1] = '\0';
    }
    g_method = method;
    g_useApi = useApi;
    g_prayerEnabled = enabled;

    // Charger les horaires manuels (fallback)
    ParseTimeString(fajr, &g_prayers[0].hour, &g_prayers[0].minute);
    ParseTimeString(dhuhr, &g_prayers[1].hour, &g_prayers[1].minute);
    ParseTimeString(asr, &g_prayers[2].hour, &g_prayers[2].minute);
    ParseTimeString(maghrib, &g_prayers[3].hour, &g_prayers[3].minute);
    ParseTimeString(isha, &g_prayers[4].hour, &g_prayers[4].minute);

    // Forcer le rafraichissement depuis l'API
    g_lastFetchDay = -1;
    if (g_useApi) {
        RefreshPrayerTimesIfNeeded();
    }
}

/*
 * SetPrayerTimes (pour compatibilite)
 * Configure uniquement les horaires manuels
 */
void SetPrayerTimes(const char* fajr, const char* dhuhr, const char* asr,
                    const char* maghrib, const char* isha, BOOL enabled) {
    ParseTimeString(fajr, &g_prayers[0].hour, &g_prayers[0].minute);
    ParseTimeString(dhuhr, &g_prayers[1].hour, &g_prayers[1].minute);
    ParseTimeString(asr, &g_prayers[2].hour, &g_prayers[2].minute);
    ParseTimeString(maghrib, &g_prayers[3].hour, &g_prayers[3].minute);
    ParseTimeString(isha, &g_prayers[4].hour, &g_prayers[4].minute);
    g_prayerEnabled = enabled;
}

/*
 * GetNextPrayer
 * Trouve la prochaine priere et calcule le temps restant
 */
static int GetNextPrayer(int* remainingMinutes) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    int currentMinutes = st.wHour * 60 + st.wMinute;

    // Chercher la prochaine priere
    for (int i = 0; i < 5; i++) {
        int prayerMinutes = g_prayers[i].hour * 60 + g_prayers[i].minute;
        if (prayerMinutes > currentMinutes) {
            *remainingMinutes = prayerMinutes - currentMinutes;
            return i;
        }
    }

    // Si toutes les prieres sont passees, retourner Fajr du lendemain
    int fajrMinutes = g_prayers[0].hour * 60 + g_prayers[0].minute;
    *remainingMinutes = (24 * 60 - currentMinutes) + fajrMinutes;
    return 0;
}

/*
 * prayer_init
 * Initialisation du plugin prayer
 */
static void prayer_init(void) {
    // Tenter de recuperer les horaires depuis l'API
    if (g_useApi) {
        RefreshPrayerTimesIfNeeded();
    }
}

/*
 * prayer_update
 * Mise a jour de l'affichage de la prochaine priere
 */
static void prayer_update(MetricData* data) {
    data->line_count = 1;

    if (!g_prayerEnabled) {
        data->enabled = FALSE;
        return;
    }

    // Rafraichir les horaires si necessaire
    RefreshPrayerTimesIfNeeded();

    int remainingMinutes = 0;
    int nextIdx = GetNextPrayer(&remainingMinutes);

    int hours = remainingMinutes / 60;
    int mins = remainingMinutes % 60;

    // Format: "Priere  Maghrib  19:00  (2h30)"
    // Ajouter indicateur si API utilisee
    char apiIndicator = g_useApi && !g_apiFailed ? '*' : ' ';

    if (hours > 0) {
        snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
                 "Priere%c %-8s %02d:%02d  (%dh%02d)",
                 apiIndicator,
                 g_prayers[nextIdx].name,
                 g_prayers[nextIdx].hour,
                 g_prayers[nextIdx].minute,
                 hours, mins);
    } else {
        snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
                 "Priere%c %-8s %02d:%02d  (%dmin)",
                 apiIndicator,
                 g_prayers[nextIdx].name,
                 g_prayers[nextIdx].hour,
                 g_prayers[nextIdx].minute,
                 mins);
    }

    // Couleur selon l'urgence
    if (remainingMinutes <= 15) {
        data->color = RGB(255, 100, 100);  // Rouge si moins de 15 min
    } else if (remainingMinutes <= 30) {
        data->color = RGB(255, 200, 100);  // Orange si moins de 30 min
    } else {
        data->color = RGB(150, 255, 200);  // Vert clair sinon
    }
}

/*
 * prayer_cleanup
 * Nettoyage du plugin prayer
 */
static void prayer_cleanup(void) {
    // Rien a nettoyer
}

/*
 * prayer_is_available
 * Verifie si le plugin est disponible
 */
static BOOL prayer_is_available(void) {
    return g_prayerEnabled;
}

// Declaration du plugin
MetricPlugin PrayerPlugin = {
    .plugin_name = "Prayer",
    .description = "Affiche la prochaine priere (API Aladhan)",
    .init = prayer_init,
    .update = prayer_update,
    .cleanup = prayer_cleanup,
    .is_available = prayer_is_available,
    .next = NULL
};
