/*
 * taskkiller.c
 * Implementation du module Task Killer
 *
 * ARCHITECTURE:
 * =============
 * 1. Enumeration via CreateToolhelp32Snapshot + Process32First/Next
 *    - Methode la plus fiable sous Windows
 *    - Ne necessite PAS de droits admin pour lire les noms
 *    - Evite les "unknown" causes par OpenProcess refuse
 *
 * 2. Kill robuste avec verification
 *    - OpenProcess avec PROCESS_TERMINATE | SYNCHRONIZE
 *    - TerminateProcess puis WaitForSingleObject
 *    - Retourne un resultat explicite (succes, acces refuse, timeout, etc.)
 *
 * 3. Protection des processus critiques
 *    - Liste noire de processus Windows essentiels
 *    - Verification PID 0 et 4
 *
 * POURQUOI CETTE APPROCHE EVITE LES "UNKNOWN":
 * ============================================
 * - OpenProcess() echoue souvent pour les processus d'autres sessions/utilisateurs
 * - CreateToolhelp32Snapshot capture TOUS les processus sans les ouvrir
 * - PROCESSENTRY32.szExeFile contient deja le nom, pas besoin d'OpenProcess
 */

#include "../include/taskkiller.h"
#include <tlhelp32.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <string.h>

// Donnees globales du Task Killer
static TaskKillerData g_taskData = {0};

// ===== LISTE DES PROCESSUS CRITIQUES =====
// Ces processus ne doivent JAMAIS etre tues car ils peuvent crasher Windows
static const char* CRITICAL_PROCESSES[] = {
    "csrss.exe",        // Client/Server Runtime - BSOD si tue
    "lsass.exe",        // Local Security Authority - BSOD si tue
    "wininit.exe",      // Windows Initialization
    "services.exe",     // Service Control Manager
    "smss.exe",         // Session Manager
    "svchost.exe",      // Service Host (certains sont critiques)
    "dwm.exe",          // Desktop Window Manager
    "winlogon.exe",     // Windows Logon
    "System",           // Kernel
    "Registry",         // Registry process
    "Memory Compression", // Memory management
    "audiodg.exe",      // Audio Device Graph (peut causer problemes)
    "fontdrvhost.exe",  // Font Driver Host
    "conhost.exe",      // Console Host
    NULL                // Terminateur
};

/*
 * IsSystemCriticalProcess
 * Verifie si un processus est dans la liste des processus critiques
 * Retourne TRUE si le processus ne doit PAS etre tue
 */
BOOL IsSystemCriticalProcess(const char* processName, DWORD pid) {
    // PID 0 = System Idle Process, PID 4 = System
    if (pid == 0 || pid == 4) {
        return TRUE;
    }

    // Verifier dans la liste des processus critiques
    for (int i = 0; CRITICAL_PROCESSES[i] != NULL; i++) {
        if (_stricmp(processName, CRITICAL_PROCESSES[i]) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * GetProcessNameByPID_Toolhelp
 * Recupere le nom d'un processus via CreateToolhelp32Snapshot
 *
 * AVANTAGE: Fonctionne meme sans droits admin car on ne fait que
 * lire un snapshot, pas ouvrir le processus
 */
BOOL GetProcessNameByPID_Toolhelp(DWORD pid, char* buffer, size_t bufferSize) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        strncpy(buffer, "unknown", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        return FALSE;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe32)) {
        CloseHandle(hSnapshot);
        strncpy(buffer, "unknown", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
        return FALSE;
    }

    BOOL found = FALSE;
    do {
        if (pe32.th32ProcessID == pid) {
            // Convertir de Wide (UTF-16) vers ANSI
            WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, -1,
                               buffer, (int)bufferSize, NULL, NULL);
            found = TRUE;
            break;
        }
    } while (Process32NextW(hSnapshot, &pe32));

    CloseHandle(hSnapshot);

    if (!found) {
        strncpy(buffer, "unknown", bufferSize - 1);
        buffer[bufferSize - 1] = '\0';
    }

    return found;
}

/*
 * TryGetFullProcessPath
 * Tente d'obtenir le chemin complet d'un processus
 * Utilise PROCESS_QUERY_LIMITED_INFORMATION qui est moins restrictif
 *
 * NOTE: Cette fonction peut echouer pour certains processus proteges,
 * mais ce n'est pas grave car on a deja le nom via Toolhelp32
 */
BOOL TryGetFullProcessPath(DWORD pid, char* buffer, size_t bufferSize) {
    buffer[0] = '\0';

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        return FALSE;
    }

    DWORD pathSize = (DWORD)bufferSize;
    BOOL success = QueryFullProcessImageNameA(hProcess, 0, buffer, &pathSize);

    CloseHandle(hProcess);
    return success;
}

/*
 * KillResultToString
 * Convertit un KillResult en message lisible pour l'utilisateur
 */
const char* KillResultToString(KillResult result) {
    switch (result) {
        case KILL_SUCCESS:          return "Process terminated";
        case KILL_ACCESS_DENIED:    return "Access denied";
        case KILL_PROTECTED_PROCESS: return "Protected process";
        case KILL_CRITICAL_PROCESS: return "Critical process!";
        case KILL_TIMEOUT:          return "Timeout (may restart)";
        case KILL_PROCESS_NOT_FOUND: return "Process not found";
        case KILL_INVALID_PID:      return "Invalid PID";
        case KILL_UNKNOWN_ERROR:    return "Unknown error";
        default:                    return "Error";
    }
}

/*
 * KillProcessAndWait
 * Tue un processus de maniere robuste avec verification
 *
 * PROCESSUS:
 * 1. Ouvre le processus avec PROCESS_TERMINATE | SYNCHRONIZE
 * 2. Appelle TerminateProcess
 * 3. Attend avec WaitForSingleObject pour confirmer la mort
 * 4. Retourne un resultat explicite
 *
 * POURQUOI CETTE APPROCHE EVITE LE FAUX REFRESH:
 * - On attend vraiment que le processus soit mort (WaitForSingleObject)
 * - Si le processus se relance automatiquement, on aura KILL_TIMEOUT
 * - Le refresh ne se fait qu'apres interpretation du resultat
 */
KillResult KillProcessAndWait(DWORD pid, DWORD timeoutMs) {
    // Verification PID invalide
    if (pid == 0 || pid == 4) {
        return KILL_INVALID_PID;
    }

    // Ouvrir le processus avec les droits necessaires
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, pid);
    if (hProcess == NULL) {
        DWORD error = GetLastError();
        if (error == ERROR_ACCESS_DENIED) {
            return KILL_ACCESS_DENIED;
        } else if (error == ERROR_INVALID_PARAMETER) {
            return KILL_PROCESS_NOT_FOUND;
        }
        return KILL_UNKNOWN_ERROR;
    }

    // Tenter de terminer le processus
    if (!TerminateProcess(hProcess, 1)) {
        DWORD error = GetLastError();
        CloseHandle(hProcess);

        if (error == ERROR_ACCESS_DENIED) {
            return KILL_PROTECTED_PROCESS;
        }
        return KILL_UNKNOWN_ERROR;
    }

    // Attendre que le processus se termine vraiment
    DWORD waitResult = WaitForSingleObject(hProcess, timeoutMs);
    CloseHandle(hProcess);

    switch (waitResult) {
        case WAIT_OBJECT_0:
            // Le processus s'est termine
            return KILL_SUCCESS;

        case WAIT_TIMEOUT:
            // Le processus n'est pas mort dans le temps imparti
            // Peut-etre un service qui se relance automatiquement
            return KILL_TIMEOUT;

        default:
            return KILL_UNKNOWN_ERROR;
    }
}

/*
 * InitTaskKiller
 * Initialise le module Task Killer
 */
void InitTaskKiller(void) {
    memset(&g_taskData, 0, sizeof(g_taskData));
    g_taskData.selected_index = -1;
    g_taskData.last_kill_result = KILL_SUCCESS;
    g_taskData.last_kill_message[0] = '\0';
    RefreshProcessList(&g_taskData);
}

/*
 * BuildProcessMapFromToolhelp
 * Construit un mapping PID -> nom de processus via Toolhelp32
 * Cette structure temporaire permet une recherche rapide
 */
typedef struct {
    DWORD pid;
    char name[MAX_PROCESS_NAME];
} ProcessMapEntry;

static int g_processMapCount = 0;
static ProcessMapEntry g_processMap[1024];  // Cache temporaire

static void BuildProcessMapFromToolhelp(void) {
    g_processMapCount = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (g_processMapCount < 1024) {
                g_processMap[g_processMapCount].pid = pe32.th32ProcessID;
                WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, -1,
                                   g_processMap[g_processMapCount].name,
                                   MAX_PROCESS_NAME, NULL, NULL);
                g_processMapCount++;
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
}

static const char* LookupProcessName(DWORD pid) {
    for (int i = 0; i < g_processMapCount; i++) {
        if (g_processMap[i].pid == pid) {
            return g_processMap[i].name;
        }
    }
    return "unknown";
}

/*
 * RefreshProcessList
 * Rafraichit la liste des processus avec ports ouverts
 *
 * PROCESSUS:
 * 1. Construire le mapping PID->nom via Toolhelp32 (une seule fois)
 * 2. Obtenir la table TCP des ports ouverts
 * 3. Pour chaque port, trouver le nom du processus dans le mapping
 */
void RefreshProcessList(TaskKillerData* data) {
    if (data == NULL) return;

    memset(data->processes, 0, sizeof(data->processes));
    data->count = 0;

    // Etape 1: Construire le mapping PID -> nom via Toolhelp32
    BuildProcessMapFromToolhelp();

    // Etape 2: Obtenir la table TCP
    PMIB_TCPTABLE_OWNER_PID pTcpTable = NULL;
    DWORD dwSize = 0;
    DWORD dwRetVal;

    dwRetVal = GetExtendedTcpTable(NULL, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_LISTENER, 0);
    if (dwRetVal != ERROR_INSUFFICIENT_BUFFER) {
        return;
    }

    pTcpTable = (PMIB_TCPTABLE_OWNER_PID)malloc(dwSize);
    if (pTcpTable == NULL) {
        return;
    }

    dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_LISTENER, 0);
    if (dwRetVal != NO_ERROR) {
        free(pTcpTable);
        return;
    }

    // Etape 3: Parcourir la table et ajouter les processus uniques
    for (DWORD i = 0; i < pTcpTable->dwNumEntries && data->count < MAX_TASK_PROCESSES; i++) {
        DWORD pid = pTcpTable->table[i].dwOwningPid;
        WORD port = ntohs((u_short)pTcpTable->table[i].dwLocalPort);

        // Ignorer PID 0 et 4 (System)
        if (pid == 0 || pid == 4) {
            continue;
        }

        // Verifier si ce PID est deja dans la liste
        BOOL found = FALSE;
        for (int j = 0; j < data->count; j++) {
            if (data->processes[j].pid == pid) {
                found = TRUE;
                break;
            }
        }

        if (!found) {
            ProcessInfo* proc = &data->processes[data->count];
            proc->pid = pid;
            proc->port = port;
            proc->is_active = TRUE;

            // Obtenir le nom depuis le mapping Toolhelp32
            const char* name = LookupProcessName(pid);
            strncpy(proc->name, name, MAX_PROCESS_NAME - 1);
            proc->name[MAX_PROCESS_NAME - 1] = '\0';

            // Optionnel: tenter d'obtenir le chemin complet
            TryGetFullProcessPath(pid, proc->path, MAX_PROCESS_PATH);

            // Marquer si processus critique
            proc->is_critical = IsSystemCriticalProcess(proc->name, pid);

            data->count++;
        }
    }

    free(pTcpTable);
}

/*
 * KillProcessByIndex
 * Tue un processus par son index dans la liste
 * Met a jour last_kill_result et last_kill_message
 *
 * NOTE: Ne rafraichit PAS automatiquement la liste
 * C'est a l'appelant de decider s'il veut rafraichir apres
 */
KillResult KillProcessByIndex(TaskKillerData* data, int index) {
    if (data == NULL || index < 0 || index >= data->count) {
        data->last_kill_result = KILL_PROCESS_NOT_FOUND;
        snprintf(data->last_kill_message, sizeof(data->last_kill_message),
                "Invalid index");
        return KILL_PROCESS_NOT_FOUND;
    }

    ProcessInfo* proc = &data->processes[index];

    // Verifier si processus critique
    if (proc->is_critical) {
        data->last_kill_result = KILL_CRITICAL_PROCESS;
        snprintf(data->last_kill_message, sizeof(data->last_kill_message),
                "Cannot kill %s (critical)", proc->name);
        return KILL_CRITICAL_PROCESS;
    }

    // Tenter de tuer le processus (timeout 3 secondes)
    KillResult result = KillProcessAndWait(proc->pid, 3000);

    data->last_kill_result = result;
    snprintf(data->last_kill_message, sizeof(data->last_kill_message),
            "%s: %s", proc->name, KillResultToString(result));

    // Rafraichir seulement si le kill a reussi
    if (result == KILL_SUCCESS) {
        RefreshProcessList(data);
    }

    return result;
}

/*
 * CleanupTaskKiller
 * Nettoie les ressources du module Task Killer
 */
void CleanupTaskKiller(void) {
    memset(&g_taskData, 0, sizeof(g_taskData));
    g_processMapCount = 0;
}

/*
 * GetTaskKillerData
 * Retourne un pointeur vers les donnees globales du Task Killer
 */
TaskKillerData* GetTaskKillerData(void) {
    return &g_taskData;
}
