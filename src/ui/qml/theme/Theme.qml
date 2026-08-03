pragma Singleton
import QtQuick
import PerformanceOverlay

// Source unique de vérité du design system (palette « cockpit » de la maquette
// FEAT - inspi full page). AUCUNE couleur / espacement / rayon codé en dur dans
// les composants : tout passe par Theme.
QtObject {
    // --- Surfaces : presque noir, panneaux à dégradé translucide ---
    readonly property color bgBase:   "#080A12"               // couche la plus profonde
    readonly property color bgBase2:  "#0B0E18"
    readonly property color bgPanel:  Qt.rgba(1, 1, 1, 0.04)  // (compat anciens composants)
    readonly property color bgRaised: Qt.rgba(1, 1, 1, 0.06)
    readonly property color panelTop: Qt.rgba(20 / 255, 25 / 255, 38 / 255, 0.82) // haut panneau
    readonly property color panelBot: Qt.rgba(11 / 255, 14 / 255, 24 / 255, 0.90) // bas panneau
    readonly property color border:   "#282D3C"               // filet 1px
    readonly property color border2:  "#1B2030"               // filet interne discret
    readonly property color stroke:     "#282D3C"             // alias (compat)
    readonly property color strokeSoft: Qt.rgba(1, 1, 1, 0.06)

    // --- Texte : hiérarchie claire ---
    readonly property color textHi:  "#E6ECF7"               // valeurs
    readonly property color text:    "#C8D2E6"               // texte courant
    readonly property color textMid: "#788296"               // labels (muted)
    readonly property color muted:   "#788296"
    readonly property color textLow: "#4A5266"               // unités/légendes (compat)
    readonly property color faint:   "#4A5266"

    // --- Accents : pilotés par le preset de thème (Config.themePreset, persistant).
    // Le couple accent/accent2 retinte TOUT (jauges, sphère, sparklines, navbar…).
    readonly property var presets: ({
        "cyan":   { accent: "#00E6FF", accent2: "#FF40B4", label: "CYAN / MAGENTA" },
        "lime":   { accent: "#46E08B", accent2: "#B4FF3D", label: "VERT / LIME" },
        "amber":  { accent: "#FFB454", accent2: "#FF5C6C", label: "AMBRE / ROUGE" },
        "violet": { accent: "#9B7BFF", accent2: "#FF5CC8", label: "VIOLET / ROSE" },
        "ice":    { accent: "#5AC8FF", accent2: "#86F4FF", label: "BLEU / GLACE" }
    })
    readonly property var presetKeys: ["cyan", "lime", "amber", "violet", "ice"]
    readonly property var _preset: presets[Config.themePreset] || presets["cyan"]

    readonly property color accent:  _preset.accent          // accent primaire (preset)
    readonly property color accent2: _preset.accent2         // accent secondaire (preset)
    readonly property color ok:      "#3DDC84"
    readonly property color warn:    "#FFA62E"
    readonly property color crit:    "#FF3B3B"

    // --- Échelle globale ---------------------------------------------------
    // Le cockpit (1280x720) et le plein écran (2560x1440) partagent la même
    // composition ; sans facteur d'échelle, les tailles en pixels fixes rendaient
    // le plein écran vide et minuscule. `scale` est affecté par Main.qml depuis la
    // hauteur de la fenêtre : les deux vues deviennent le MÊME écran à deux
    // tailles. Borné en bas à 1 pour que le mode compact (560 px) ne rapetisse
    // pas le texte, en haut pour éviter le gigantisme sur un écran très haut.
    property real scale: 1.0
    function px(v) { return Math.round(v * scale) }

    // --- Géométrie : cockpit = coins quasi nets ---
    readonly property real radius:   3
    readonly property real radiusSm: 2
    readonly property real gap:      px(16)   // unité de base ; multiplier, ne pas inventer
    readonly property real pad:      px(16)

    // --- Typographie : tout en Satoshi. Les chiffres restent alignés grâce à la
    // feature tnum activée globalement (main.cpp) → pas de sautillement.
    readonly property string fontUi:   "Satoshi"
    readonly property string fontMono: "Satoshi"

    // Échelle typographique : SEULES ces tailles doivent être utilisées. Avant,
    // chaque zone inventait la sienne (9, 10, 11, 12, 13, 14, 15, 16, 22, 24,
    // 26, 28, 30…) — d'où l'impression d'un assemblage hétérogène.
    readonly property real fsMicro: px(9)    // légendes, unités, mentions
    readonly property real fsLabel: px(10)   // libellés de ligne (majuscules)
    readonly property real fsBody:  px(12)   // texte courant
    readonly property real fsTitle: px(13)   // titre de panneau
    readonly property real fsValue: px(14)   // valeur numérique d'une ligne
    readonly property real fsLead:  px(20)   // valeur mise en avant d'une tuile
    readonly property real fsBig:   px(26)   // valeur héroïque (chrono, météo, ping)

    // Interlettrage : deux valeurs, pas huit.
    readonly property real lsLabel: 1.4      // libellés
    readonly property real lsTitle: 2.4      // titres de panneau

    // Mappe une charge 0..1 vers une couleur sémantique.
    function statusColor(t) {
        return t > 0.9 ? crit : t > 0.72 ? warn : ok
    }
}
