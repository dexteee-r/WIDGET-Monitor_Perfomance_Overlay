# Guide Pédagogique - Apprendre le C avec Performance Overlay

## Table des Matières
1. [Introduction au Langage C](#1-introduction-au-langage-c)
2. [Structure d'un Programme C](#2-structure-dun-programme-c)
3. [Les Types de Données](#3-les-types-de-données)
4. [Les Pointeurs (Concept Clé)](#4-les-pointeurs-concept-clé)
5. [Les Structures](#5-les-structures)
6. [Les Fonctions](#6-les-fonctions)
7. [La Programmation Windows (Win32 API)](#7-la-programmation-windows-win32-api)
8. [Analyse du Code - Module par Module](#8-analyse-du-code---module-par-module)
9. [Exercices Pratiques](#9-exercices-pratiques)

---

## 1. Introduction au Langage C

### Pourquoi le C ?
- **Rapide** : Code compilé directement en langage machine
- **Léger** : Pas de runtime lourd (contrairement à Python/Java)
- **Contrôle** : Accès direct à la mémoire et au matériel
- **Universel** : Base de Windows, Linux, macOS

### Comparaison Python vs C

**Python (votre code original):**
```python
import tkinter as tk
cpu_usage = psutil.cpu_percent()
```

**C (nouvelle version):**
```c
#include <windows.h>
float cpu_usage = GetCPUUsage();
```

**Différences clés:**
- C nécessite la **compilation** (transformation en .exe)
- C demande une gestion **manuelle** de la mémoire
- C est **beaucoup plus rapide** (10-100x selon les cas)

---

## 2. Structure d'un Programme C

### Anatomie d'un fichier C

```c
// 1. DIRECTIVES DE PRÉPROCESSEUR (avant compilation)
#include <stdio.h>      // Inclure des bibliothèques
#define MAX_SIZE 100    // Définir des constantes

// 2. DÉCLARATIONS GLOBALES
int compteur = 0;       // Variable globale

// 3. PROTOTYPES DE FONCTIONS
void afficherMessage();

// 4. FONCTION PRINCIPALE
int main() {
    printf("Hello World!");
    return 0;
}

// 5. IMPLÉMENTATION DES FONCTIONS
void afficherMessage() {
    printf("Bonjour!\n");
}
```

### Le Préprocesseur
Les lignes commençant par `#` sont traitées **avant** la compilation :

```c
#include <stdio.h>     // Copie le contenu de stdio.h ici
#include "config.h"    // Inclut votre propre fichier .h

#define PI 3.14159     // Remplace tous les "PI" par "3.14159"

#ifdef _WIN32          // Code conditionnel (seulement sur Windows)
    #include <windows.h>
#endif
```

---

## 3. Les Types de Données

### Types de Base

```c
// ENTIERS
int age = 25;                    // Entier signé (-2 milliards à +2 milliards)
unsigned int positif = 100;      // Seulement positif (0 à 4 milliards)
short petit = 10;                // Petit entier (-32768 à 32767)
long grand = 1000000L;           // Grand entier

// FLOTTANTS
float temperature = 36.5f;       // Nombre à virgule (précision ~7 chiffres)
double precis = 3.141592653589;  // Double précision (~15 chiffres)

// CARACTÈRES
char lettre = 'A';               // Un seul caractère
char texte[] = "Bonjour";        // Tableau de caractères (chaîne)

// BOOLÉENS (en C)
BOOL estVrai = TRUE;             // Dans Win32 API
int condition = 1;               // 0 = faux, tout le reste = vrai
```

### Tailles en Mémoire

```c
sizeof(char)      // 1 octet
sizeof(int)       // 4 octets (généralement)
sizeof(float)     // 4 octets
sizeof(double)    // 8 octets
sizeof(void*)     // 4 ou 8 octets (selon architecture 32/64 bits)
```

---

## 4. Les Pointeurs (Concept Clé)

### Qu'est-ce qu'un Pointeur ?

**Analogie:** Un pointeur est comme une **adresse postale** qui indique **où** se trouve une donnée en mémoire.

```c
int age = 25;        // Variable normale (contient la valeur 25)
int* ptr = &age;     // Pointeur (contient l'ADRESSE de age)

// Opérateurs
&age     // Opérateur "adresse de" → donne l'adresse mémoire
*ptr     // Opérateur "déréférencement" → donne la valeur à l'adresse
```

### Exemple Concret

```c
void incrementer(int* nombre) {
    *nombre = *nombre + 1;    // Modifie la valeur à l'adresse
}

int main() {
    int score = 10;
    incrementer(&score);      // Passe l'ADRESSE de score
    printf("%d", score);      // Affiche 11 (modifié!)
    return 0;
}
```

**Pourquoi c'est important ?**
- Permet de modifier des variables dans d'autres fonctions
- Évite de copier de grosses données
- Nécessaire pour les structures dynamiques

### Dans Notre Programme

```c
void GetPerformanceData(PerformanceData* data) {
    data->cpu_usage = GetCPUUsage();  // Modifie directement la structure
}

// Utilisation
PerformanceData perf;
GetPerformanceData(&perf);  // On passe l'adresse
```

---

## 5. Les Structures

### Créer des Types Personnalisés

En Python, vous utilisez des classes. En C, on utilise des **structures** :

```c
// DÉFINITION
typedef struct {
    float cpu_usage;
    float memory_usage;
    float disk_usage;
    char disk_name[32];
} PerformanceData;

// UTILISATION
PerformanceData perf;           // Créer une variable
perf.cpu_usage = 45.2f;         // Accéder aux membres avec '.'

PerformanceData* ptr = &perf;
ptr->cpu_usage = 50.0f;         // Avec pointeur, utiliser '->'
```

### typedef
`typedef` crée un **alias** pour un type :

```c
typedef unsigned int uint;      // uint est maintenant un alias
uint nombre = 42;               // Au lieu de "unsigned int nombre"

typedef struct {
    int x, y;
} Point;

Point p;  // Au lieu de "struct Point p"
```

---

## 6. Les Fonctions

### Syntaxe de Base

```c
// TYPE_RETOUR nom_fonction(TYPE param1, TYPE param2) {
//     corps de la fonction
//     return valeur;
// }

float calculerMoyenne(float a, float b) {
    return (a + b) / 2.0f;
}

void afficherTexte(const char* texte) {
    printf("%s\n", texte);
    // void = pas de return
}
```

### Passage par Valeur vs Référence

```c
// PASSAGE PAR VALEUR (copie)
void doubler(int x) {
    x = x * 2;    // Modifie seulement la COPIE
}

int a = 5;
doubler(a);
// a vaut toujours 5

// PASSAGE PAR RÉFÉRENCE (pointeur)
void doublerRef(int* x) {
    *x = *x * 2;  // Modifie l'ORIGINAL
}

int b = 5;
doublerRef(&b);
// b vaut maintenant 10
```

---

## 7. La Programmation Windows (Win32 API)

### Qu'est-ce que Win32 API ?

C'est la **bibliothèque native de Windows** pour créer des applications. Pas besoin de Tkinter ou autre !

### Concepts de Base

#### 1. HWND - Handle de Fenêtre
```c
HWND hwnd;  // "Handle to Window" = identifiant unique d'une fenêtre
```

C'est comme un **numéro d'identification** pour votre fenêtre.

#### 2. Messages Windows
Windows fonctionne sur un système de **messages** :

```c
WM_CREATE    // Fenêtre créée
WM_PAINT     // Besoin de redessiner
WM_TIMER     // Timer déclenché
WM_LBUTTONDOWN // Clic gauche souris
WM_KEYDOWN   // Touche pressée
WM_DESTROY   // Fenêtre fermée
```

#### 3. Boucle de Messages
```c
MSG msg;
while (GetMessage(&msg, NULL, 0, 0)) {  // Récupère un message
    TranslateMessage(&msg);              // Traduit les touches
    DispatchMessage(&msg);               // Envoie au WindowProc
}
```

#### 4. WindowProc - Le Cerveau de l'Interface
```c
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
                             WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT:
            // Dessiner la fenêtre
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
```

### Créer une Fenêtre Simple

```c
// 1. Enregistrer la classe de fenêtre
WNDCLASS wc = {0};
wc.lpfnWndProc = WindowProc;        // Fonction de traitement
wc.hInstance = hInstance;
wc.lpszClassName = "MaClasse";
RegisterClass(&wc);

// 2. Créer la fenêtre
HWND hwnd = CreateWindowEx(
    0,                      // Styles étendus
    "MaClasse",            // Nom de classe
    "Ma Fenêtre",          // Titre
    WS_OVERLAPPEDWINDOW,   // Style
    100, 100,              // Position x, y
    400, 300,              // Largeur, Hauteur
    NULL, NULL, hInstance, NULL
);

// 3. Afficher
ShowWindow(hwnd, SW_SHOW);
```

---

## 8. Analyse du Code - Module par Module

### Module 1: performance.c - Monitoring Système

#### Fonction InitPerformanceMonitoring()

```c
void InitPerformanceMonitoring() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);  // Obtient infos système
    numProcessors = sysInfo.dwNumberOfProcessors;

    // ...initialisation des compteurs CPU
}
```

**Explication :**
1. `SYSTEM_INFO` est une **structure Windows** contenant les infos hardware
2. `GetSystemInfo()` est une **fonction Win32** qui remplit cette structure
3. On stocke le nombre de processeurs pour calculer le CPU plus tard

#### Fonction GetCPUUsage()

```c
static float GetCPUUsage() {
    FILETIME ftime, fsys, fuser;

    // Obtenir le temps actuel
    GetSystemTimeAsFileTime(&ftime);

    // Obtenir les temps CPU
    GetProcessTimes(self, &ftime, &ftime, &fsys, &fuser);

    // Calculer la différence depuis le dernier appel
    percent = (sys.QuadPart - lastSysCPU.QuadPart) +
              (user.QuadPart - lastUserCPU.QuadPart);
    percent /= (now.QuadPart - lastCPU.QuadPart);

    return (float)(percent * 100.0);
}
```

**Concepts clés :**
- `static` = variable/fonction **locale au fichier**
- `FILETIME` = structure représentant le temps Windows
- `QuadPart` = partie 64-bit d'un entier (pour grands nombres)
- Calcul du % = (temps utilisé) / (temps écoulé) × 100

#### Fonction GetMemoryUsage()

```c
static float GetMemoryUsage() {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);  // Important!

    GlobalMemoryStatusEx(&memInfo);

    return (float)memInfo.dwMemoryLoad;  // Déjà en %
}
```

**Note importante :** Beaucoup de structures Win32 demandent qu'on initialise leur taille avec `dwLength` avant de les utiliser.

---

### Module 2: config.c - Gestion Configuration

#### Fonction LoadConfig()

```c
void LoadConfig(Config* config) {
    FILE* file = fopen(CONFIG_FILE, "r");  // "r" = lecture

    if (file != NULL) {
        // Lire depuis le fichier
        fscanf(file, "x=%d\n", &config->x);
        fscanf(file, "y=%d\n", &config->y);

        int minimal;
        fscanf(file, "minimal_mode=%d\n", &minimal);
        config->minimal_mode = (minimal != 0);

        fclose(file);
    } else {
        // Valeurs par défaut
        config->x = 10;
        config->y = 10;
        config->minimal_mode = FALSE;
    }
}
```

**Explication :**
- `fopen()` ouvre un fichier (retourne NULL si échec)
- `fscanf()` lit des données formatées (comme scanf mais depuis un fichier)
- `%d` = format pour un entier
- `&config->x` = adresse du membre x de la structure

#### Fonction SaveConfig()

```c
void SaveConfig(const Config* config) {
    FILE* file = fopen(CONFIG_FILE, "w");  // "w" = écriture

    if (file != NULL) {
        fprintf(file, "x=%d\n", config->x);
        fprintf(file, "y=%d\n", config->y);
        fprintf(file, "minimal_mode=%d\n",
                config->minimal_mode ? 1 : 0);

        fclose(file);
    }
}
```

**Note :** `const Config*` signifie qu'on **ne peut pas modifier** la structure. C'est une bonne pratique pour la sécurité.

---

### Module 3: startup.c - Démarrage Automatique

```c
BOOL AddToStartup() {
    HKEY hKey;
    char exePath[MAX_PATH];

    // 1. Obtenir le chemin de notre .exe
    GetModuleFileName(NULL, exePath, MAX_PATH);

    // 2. Ouvrir la clé de registre
    RegOpenKeyEx(
        HKEY_CURRENT_USER,  // Registre utilisateur
        "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0,
        KEY_SET_VALUE,       // Permission d'écriture
        &hKey
    );

    // 3. Ajouter une valeur
    RegSetValueEx(
        hKey,
        "PerformanceOverlay",  // Nom
        0,
        REG_SZ,                 // Type: chaîne
        (BYTE*)exePath,
        strlen(exePath) + 1
    );

    RegCloseKey(hKey);
}
```

**Concepts :**
- **Registre Windows** = base de données système pour configuration
- `HKEY` = Handle vers une clé de registre
- `MAX_PATH` = constante Windows (260 caractères max)

---

### Module 4: main.c - Programme Principal

#### WindowProc - Le Switch Géant

```c
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
                             WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // Fenêtre vient d'être créée
            SetTimer(hwnd, TIMER_ID, 2000, NULL);
            return 0;

        case WM_TIMER:
            // Timer déclenché (toutes les 2 secondes)
            GetPerformanceData(&g_perfData);
            UpdateDisplay();
            return 0;

        case WM_PAINT: {
            // Il faut redessiner la fenêtre
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // ... dessiner le texte

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            // Clic gauche
            POINT pt = {LOWORD(lParam), HIWORD(lParam)};

            // Vérifier si c'est sur le bouton X
            if (pt.x >= 5 && pt.x <= 25 && pt.y >= 5 && pt.y <= 25) {
                PostQuitMessage(0);  // Quitter
            } else {
                // Commencer le drag
                g_isDragging = TRUE;
            }
            return 0;
        }

        case WM_KEYDOWN:
            // Touche pressée
            if (wParam == VK_F1) {
                // Toggle visibilité
            }
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
```

**Vocabulaire :**
- `LRESULT` = type de retour (long)
- `CALLBACK` = convention d'appel (comment Windows appelle la fonction)
- `WPARAM` et `LPARAM` = paramètres supplémentaires du message
- `LOWORD()` / `HIWORD()` = extrait les 16 bits bas/haut

#### Dessiner du Texte

```c
case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);  // HDC = Handle Device Context

    // Créer une police
    HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL,
                             FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                             DEFAULT_PITCH | FF_SWISS, "Arial");

    SelectObject(hdc, hFont);              // Sélectionner la police
    SetTextColor(hdc, RGB(255,255,255));   // Couleur blanche
    SetBkMode(hdc, TRANSPARENT);           // Fond transparent

    // Préparer le texte
    char text[256];
    sprintf(text, "CPU: %.1f%%\nRAM: %.1f%%",
            g_perfData.cpu_usage,
            g_perfData.memory_usage);

    // Dessiner
    RECT rect = {30, 5, 300, 200};
    DrawText(hdc, text, -1, &rect, DT_LEFT | DT_TOP);

    DeleteObject(hFont);      // Libérer la police
    EndPaint(hwnd, &ps);
}
```

**Concepts GDI (Graphics Device Interface) :**
- `HDC` = contexte de dessin (comme un "pinceau")
- `CreateFont()` = créer une police
- `SelectObject()` = choisir l'outil à utiliser
- `DrawText()` = dessiner du texte
- **Important :** Toujours `DeleteObject()` pour libérer la mémoire !

#### Fenêtre Transparente

```c
CreateWindowEx(
    WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
    // WS_EX_LAYERED = permet transparence
    // WS_EX_TOPMOST = toujours au premier plan
    // WS_EX_TOOLWINDOW = pas dans la barre des tâches
    ...
);

SetLayeredWindowAttributes(
    hwnd,
    RGB(0, 0, 0),  // Couleur à rendre transparente (noir)
    230,            // Alpha (0=invisible, 255=opaque)
    LWA_COLORKEY | LWA_ALPHA
);
```

---

## 9. Exercices Pratiques

### Niveau Débutant

**Exercice 1 : Modifier la couleur du texte**
Changez la couleur du texte de blanc à vert :
```c
// Dans WM_PAINT
SetTextColor(hdc, RGB(0, 255, 0));  // Rouge, Vert, Bleu
```

**Exercice 2 : Changer l'intervalle de mise à jour**
Modifiez le timer pour une mise à jour toutes les 1 seconde :
```c
#define TIMER_INTERVAL 1000  // En millisecondes
```

**Exercice 3 : Ajouter une nouvelle métrique**
Ajoutez l'affichage de la RAM totale en GB dans `GetMemoryUsage()`.

### Niveau Intermédiaire

**Exercice 4 : Ajouter un bouton "Mode Minimal"**
Créez un bouton qui bascule le mode minimal (sans utiliser F2).

**Exercice 5 : Seuils d'alerte**
Changez la couleur du texte en rouge si CPU > 80%.

**Exercice 6 : Historique**
Gardez un historique des 10 dernières valeurs CPU et affichez la moyenne.

### Niveau Avancé

**Exercice 7 : Graphique**
Dessinez un petit graphique de l'utilisation CPU avec `LineTo()`.

**Exercice 8 : Multi-moniteurs**
Adaptez le code pour fonctionner sur plusieurs écrans.

**Exercice 9 : Optimisation**
Réduisez l'utilisation CPU du programme lui-même à < 1%.

---

## Vocabulaire Important

| Terme | Signification |
|-------|---------------|
| **Pointeur** | Variable contenant une adresse mémoire |
| **Structure** | Type de données personnalisé groupant plusieurs variables |
| **Handle** | Identifiant unique d'une ressource Windows (fenêtre, fichier, etc.) |
| **API** | Application Programming Interface (fonctions fournies par le système) |
| **Callback** | Fonction appelée automatiquement par le système |
| **GDI** | Graphics Device Interface (API de dessin Windows) |
| **Heap** | Zone mémoire pour allocation dynamique |
| **Stack** | Pile d'exécution (variables locales) |

---

## Conseils pour Débuter

1. **Compilez souvent** : Détectez les erreurs tôt
2. **Utilisez printf()** : Pour déboguer (afficher des valeurs)
3. **Lisez les erreurs** : Le compilateur vous aide !
4. **Commentez votre code** : Vous-même dans 1 mois serez reconnaissant
5. **Commencez petit** : Maîtrisez les bases avant d'aller plus loin

---

## Ressources Complémentaires

### Documentation
- [Documentation Win32 Microsoft](https://learn.microsoft.com/en-us/windows/win32/)
- [cppreference.com](https://en.cppreference.com/w/c) - Référence C complète

### Tutoriels
- "C Programming Absolute Beginner's Guide"
- "Windows Programming" par Charles Petzold (bible du Win32)

### Outils
- **Compilateur** : MinGW-w64, Visual Studio
- **Éditeur** : VS Code, Code::Blocks, Dev-C++
- **Débogueur** : GDB, Visual Studio Debugger

---

## Prochaines Étapes

1. Compilez et testez le programme
2. Faites les exercices niveau débutant
3. Modifiez le code pour ajouter vos propres fonctionnalités
4. Explorez d'autres API Windows (OpenGL pour 3D, DirectX, etc.)
5. Apprenez le C++ pour aller plus loin

**Bon apprentissage !**
