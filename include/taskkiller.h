/*
 * taskkiller.h
 * Module Task Killer - Liste et tue les processus avec ports ouverts
 *
 * Architecture:
 * - Enumeration via CreateToolhelp32Snapshot (fiable, pas besoin de droits admin)
 * - Kill robuste avec WaitForSingleObject et gestion des erreurs
 * - Protection des processus systeme critiques
 */

#ifndef TASKKILLER_H
#define TASKKILLER_H

#include <windows.h>

#define MAX_TASK_PROCESSES 100
#define MAX_PROCESS_NAME 64
#define MAX_PROCESS_PATH 260

// Modes d'affichage
typedef enum {
    VIEW_MODE_PORTS = 0,    // Seulement les processus avec ports
    VIEW_MODE_ALL           // Tous les processus (top CPU/RAM)
} TaskViewMode;

// Resultat d'une operation Kill
typedef enum {
    KILL_SUCCESS = 0,           // Processus tue avec succes
    KILL_ACCESS_DENIED,         // Acces refuse (droits insuffisants)
    KILL_PROTECTED_PROCESS,     // Processus protege par le systeme
    KILL_CRITICAL_PROCESS,      // Processus critique (interdit de tuer)
    KILL_TIMEOUT,               // Timeout - processus peut-etre relance
    KILL_PROCESS_NOT_FOUND,     // Processus introuvable
    KILL_INVALID_PID,           // PID invalide (0, 4, etc.)
    KILL_UNKNOWN_ERROR          // Erreur inconnue
} KillResult;

// Structure pour un processus avec port
typedef struct {
    DWORD pid;                          // Process ID
    char name[MAX_PROCESS_NAME];        // Nom du processus (ex: "node.exe")
    char path[MAX_PROCESS_PATH];        // Chemin complet (optionnel)
    WORD port;                          // Port utilise (0 si aucun)
    BOOL is_active;                     // Processus actif
    BOOL is_critical;                   // Processus critique (ne pas tuer)
    float cpu_percent;                  // Usage CPU en %
    SIZE_T memory_mb;                   // Usage RAM en MB
} ProcessInfo;

// Structure pour la liste des processus
typedef struct {
    ProcessInfo processes[MAX_TASK_PROCESSES];
    int count;                          // Nombre de processus
    int selected_index;                 // Index selectionne (-1 si aucun)
    int scroll_offset;                  // Offset de scroll
    KillResult last_kill_result;        // Resultat du dernier kill
    char last_kill_message[128];        // Message du dernier kill
    // Filtre de recherche
    char filter_text[32];               // Texte de filtre (nom ou port)
    BOOL filter_active;                 // Filtre actif (champ selectionne)
    // Mode d'affichage
    TaskViewMode view_mode;             // VIEW_MODE_PORTS ou VIEW_MODE_ALL
} TaskKillerData;

// ===== FONCTIONS PRINCIPALES =====

// Initialisation et nettoyage
void InitTaskKiller(void);
void CleanupTaskKiller(void);

// Enumeration des processus (via Toolhelp32)
void RefreshProcessList(TaskKillerData* data);

// Rafraichit selon le mode actuel (ports ou tous)
void RefreshCurrentView(TaskKillerData* data);

// Toggle entre les modes
void ToggleViewMode(TaskKillerData* data);

// ===== FONCTIONS DE KILL =====

// Tue un processus par son PID avec gestion complete des erreurs
// Retourne un KillResult indiquant le resultat de l'operation
KillResult KillProcessAndWait(DWORD pid, DWORD timeoutMs);

// Tue un processus par son index dans la liste
// Met a jour last_kill_result et last_kill_message
KillResult KillProcessByIndex(TaskKillerData* data, int index);

// ===== FONCTIONS UTILITAIRES =====

// Verifie si un processus est critique (interdit de tuer)
BOOL IsSystemCriticalProcess(const char* processName, DWORD pid);

// Obtient le nom d'un processus par son PID (via Toolhelp32)
BOOL GetProcessNameByPID_Toolhelp(DWORD pid, char* buffer, size_t bufferSize);

// Tente d'obtenir le chemin complet d'un processus
BOOL TryGetFullProcessPath(DWORD pid, char* buffer, size_t bufferSize);

// Convertit un KillResult en message lisible
const char* KillResultToString(KillResult result);

// Donnees globales
TaskKillerData* GetTaskKillerData(void);

// ===== FONCTIONS DE FILTRAGE =====

// Verifie si un processus correspond au filtre (nom ou port)
BOOL MatchesFilter(ProcessInfo* proc, const char* filter);

// Retourne le nombre de processus apres filtrage
int GetFilteredProcessCount(TaskKillerData* data);

// Retourne un processus par son index visible (apres filtrage)
ProcessInfo* GetFilteredProcessByVisibleIndex(TaskKillerData* data, int visibleIndex);

// Convertit un index visible en index reel
int GetRealIndexFromVisibleIndex(TaskKillerData* data, int visibleIndex);

#endif // TASKKILLER_H
