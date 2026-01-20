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

#define MAX_TASK_PROCESSES 50
#define MAX_PROCESS_NAME 64
#define MAX_PROCESS_PATH 260

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
} ProcessInfo;

// Structure pour la liste des processus
typedef struct {
    ProcessInfo processes[MAX_TASK_PROCESSES];
    int count;                          // Nombre de processus
    int selected_index;                 // Index selectionne (-1 si aucun)
    int scroll_offset;                  // Offset de scroll
    KillResult last_kill_result;        // Resultat du dernier kill
    char last_kill_message[128];        // Message du dernier kill
} TaskKillerData;

// ===== FONCTIONS PRINCIPALES =====

// Initialisation et nettoyage
void InitTaskKiller(void);
void CleanupTaskKiller(void);

// Enumeration des processus (via Toolhelp32)
void RefreshProcessList(TaskKillerData* data);

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

#endif // TASKKILLER_H
