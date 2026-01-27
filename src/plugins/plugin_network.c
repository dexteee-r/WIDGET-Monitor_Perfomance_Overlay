/*
 * plugin_network.c
 * Plugin pour afficher l'adresse IP locale
 */

#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

// Note: Les libs sont linkées via Makefile (-liphlpapi -lws2_32)

// Cache pour l'IP (pas besoin de recalculer à chaque frame)
static char cachedIP[64] = "N/A";
static DWORD lastRefreshTick = 0;
#define IP_REFRESH_INTERVAL 30000  // Rafraîchir toutes les 30 secondes

/*
 * GetLocalIPAddress
 * Obtient l'adresse IP locale principale (pas 127.0.0.1)
 */
static void GetLocalIPAddress(char* buffer, size_t bufferSize) {
    ULONG bufLen = 0;
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    PIP_ADAPTER_ADDRESSES pCurr = NULL;

    // Premier appel pour obtenir la taille nécessaire
    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST,
                             NULL, NULL, &bufLen) != ERROR_BUFFER_OVERFLOW) {
        strncpy(buffer, "N/A", bufferSize);
        return;
    }

    pAddresses = (IP_ADAPTER_ADDRESSES*)malloc(bufLen);
    if (!pAddresses) {
        strncpy(buffer, "N/A", bufferSize);
        return;
    }

    // Deuxième appel pour obtenir les données
    if (GetAdaptersAddresses(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST,
                             NULL, pAddresses, &bufLen) != NO_ERROR) {
        free(pAddresses);
        strncpy(buffer, "N/A", bufferSize);
        return;
    }

    // Parcourir les adaptateurs pour trouver une IP valide
    for (pCurr = pAddresses; pCurr != NULL; pCurr = pCurr->Next) {
        // Ignorer les adaptateurs désactivés ou loopback
        if (pCurr->OperStatus != IfOperStatusUp) continue;
        if (pCurr->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;

        // Chercher une adresse unicast IPv4
        PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurr->FirstUnicastAddress;
        while (pUnicast) {
            if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                struct sockaddr_in* sa_in = (struct sockaddr_in*)pUnicast->Address.lpSockaddr;
                char* ip = inet_ntoa(sa_in->sin_addr);

                // Ignorer 127.0.0.1 et les adresses APIPA (169.254.x.x)
                if (strcmp(ip, "127.0.0.1") != 0 &&
                    strncmp(ip, "169.254.", 8) != 0) {
                    strncpy(buffer, ip, bufferSize - 1);
                    buffer[bufferSize - 1] = '\0';
                    free(pAddresses);
                    return;
                }
            }
            pUnicast = pUnicast->Next;
        }
    }

    free(pAddresses);
    strncpy(buffer, "No Network", bufferSize);
}

/*
 * network_init
 * Initialisation du plugin réseau
 */
static void network_init(void) {
    // Initialiser Winsock si nécessaire
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Obtenir l'IP au démarrage
    GetLocalIPAddress(cachedIP, sizeof(cachedIP));
    lastRefreshTick = GetTickCount();
}

/*
 * network_update
 * Mise à jour des données réseau
 */
static void network_update(MetricData* data) {
    // Rafraîchir l'IP périodiquement
    DWORD currentTick = GetTickCount();
    if (currentTick - lastRefreshTick > IP_REFRESH_INTERVAL) {
        GetLocalIPAddress(cachedIP, sizeof(cachedIP));
        lastRefreshTick = currentTick;
    }

    data->value = 0;  // Pas de valeur numérique
    data->line_count = 1;

    // Formater la ligne d'affichage
    snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
             "IP    %s", cachedIP);

    // Couleur
    if (strcmp(cachedIP, "No Network") == 0 || strcmp(cachedIP, "N/A") == 0) {
        data->color = RGB(255, 100, 100);  // Rouge si pas de réseau
    } else {
        data->color = COLOR_CYAN;  // Cyan normal
    }
}

/*
 * network_cleanup
 * Nettoyage du plugin réseau
 */
static void network_cleanup(void) {
    WSACleanup();
}

/*
 * network_is_available
 * Vérifier si le réseau est disponible
 */
static BOOL network_is_available(void) {
    return TRUE;  // Toujours disponible
}

// Déclaration du plugin
MetricPlugin NetworkPlugin = {
    .plugin_name = "Network",
    .description = "Affiche l'adresse IP locale",
    .init = network_init,
    .update = network_update,
    .cleanup = network_cleanup,
    .is_available = network_is_available,
    .next = NULL
};
