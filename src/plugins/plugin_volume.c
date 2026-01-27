/*
 * plugin_volume.c
 * Plugin pour afficher le volume système (Windows Vista+)
 */

// INITGUID doit être défini AVANT tous les includes pour que les GUIDs soient définis
#define INITGUID
#include <initguid.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include "../../include/metric_plugin.h"
#include "../../include/constants.h"

// Interfaces COM pour le volume
static IMMDeviceEnumerator* pEnumerator = NULL;
static IMMDevice* pDevice = NULL;
static IAudioEndpointVolume* pVolume = NULL;
static BOOL comInitialized = FALSE;

/*
 * BuildVolumeBar
 * Construit une barre de volume ASCII
 */
static void BuildVolumeBar(char* buffer, size_t len, float percent, int width) {
    if (len == 0 || (int)len <= width + 3) {
        if (len > 0) buffer[0] = '\0';
        return;
    }

    if (percent < 0.0f) percent = 0.0f;
    if (percent > 100.0f) percent = 100.0f;

    int filled = (int)((percent / 100.0f) * width + 0.5f);
    if (filled > width) filled = width;

    buffer[0] = '[';
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            buffer[1 + i] = '#';
        } else {
            buffer[1 + i] = '-';
        }
    }
    buffer[1 + width] = ']';
    buffer[2 + width] = '\0';
}

/*
 * volume_init
 * Initialisation du plugin volume via Windows Core Audio API
 */
static void volume_init(void) {
    HRESULT hr;

    // Initialiser COM
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hr) || hr == S_FALSE) {
        comInitialized = TRUE;
    }

    // Créer l'enumerator de devices
    hr = CoCreateInstance(
        &CLSID_MMDeviceEnumerator,
        NULL,
        CLSCTX_ALL,
        &IID_IMMDeviceEnumerator,
        (void**)&pEnumerator
    );

    if (FAILED(hr) || !pEnumerator) {
        return;
    }

    // Obtenir le device audio par défaut
    hr = pEnumerator->lpVtbl->GetDefaultAudioEndpoint(
        pEnumerator,
        eRender,
        eConsole,
        &pDevice
    );

    if (FAILED(hr) || !pDevice) {
        return;
    }

    // Obtenir l'interface de contrôle du volume
    hr = pDevice->lpVtbl->Activate(
        pDevice,
        &IID_IAudioEndpointVolume,
        CLSCTX_ALL,
        NULL,
        (void**)&pVolume
    );
}

/*
 * volume_update
 * Mise à jour du volume système
 */
static void volume_update(MetricData* data) {
    float volumeLevel = 0.0f;
    BOOL isMuted = FALSE;

    data->line_count = 1;

    if (pVolume) {
        pVolume->lpVtbl->GetMasterVolumeLevelScalar(pVolume, &volumeLevel);
        pVolume->lpVtbl->GetMute(pVolume, &isMuted);
    }

    float percent = volumeLevel * 100.0f;
    data->value = percent;

    // Construire la barre de volume
    char bar[32];
    BuildVolumeBar(bar, sizeof(bar), percent, 15);

    // Formater l'affichage
    if (isMuted) {
        snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
                 "Vol   MUTED     %s", bar);
        data->color = RGB(255, 100, 100);  // Rouge si muet
    } else {
        snprintf(data->display_lines[0], sizeof(data->display_lines[0]),
                 "Vol   %5.1f%%    %s", percent, bar);

        // Couleur selon le niveau
        if (percent > 80.0f) {
            data->color = RGB(255, 150, 100);  // Orange si fort
        } else if (percent > 50.0f) {
            data->color = COLOR_TEXT_PRIMARY;
        } else {
            data->color = RGB(150, 200, 255);  // Bleu si faible
        }
    }
}

/*
 * volume_cleanup
 * Nettoyage du plugin volume
 */
static void volume_cleanup(void) {
    if (pVolume) {
        pVolume->lpVtbl->Release(pVolume);
        pVolume = NULL;
    }
    if (pDevice) {
        pDevice->lpVtbl->Release(pDevice);
        pDevice = NULL;
    }
    if (pEnumerator) {
        pEnumerator->lpVtbl->Release(pEnumerator);
        pEnumerator = NULL;
    }
    if (comInitialized) {
        CoUninitialize();
        comInitialized = FALSE;
    }
}

/*
 * volume_is_available
 * Vérifier si le contrôle du volume est disponible
 */
static BOOL volume_is_available(void) {
    return (pVolume != NULL);
}

// Déclaration du plugin
MetricPlugin VolumePlugin = {
    .plugin_name = "Volume",
    .description = "Affiche le volume système",
    .init = volume_init,
    .update = volume_update,
    .cleanup = volume_cleanup,
    .is_available = volume_is_available,
    .next = NULL
};
