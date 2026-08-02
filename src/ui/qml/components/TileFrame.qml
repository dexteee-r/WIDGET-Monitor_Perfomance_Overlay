import QtQuick
import PerformanceOverlay

// Enveloppe d'une zone posée sur la grille libre du cockpit.
//
// La géométrie vient du singleton `Layouts` (en cellules) ; en mode édition, la
// tuile se déplace et se redimensionne à la souris puis s'aligne sur la grille
// au relâchement. Pendant le geste on décale la tuile par un OFFSET en pixels
// plutôt que d'écrire dans le modèle à chaque frame : le rendu suit la souris au
// pixel près, et le modèle ne reçoit qu'une seule écriture (donc une seule
// sauvegarde dans config.ini) à la fin du geste.
Item {
    id: frame

    property string tileId: ""
    property real cellW: 1
    property real cellH: 1
    property real gap: Theme.gap

    default property alias content: body.data

    readonly property var g: Layouts.tiles[frame.tileId] !== undefined
                             ? Layouts.tiles[frame.tileId]
                             : [0, 0, 4, 4]

    // Décalages du geste en cours (remis à zéro au relâchement).
    property real offX: 0
    property real offY: 0
    property real offW: 0
    property real offH: 0
    readonly property bool busy: moveArea.pressed || sizeArea.pressed

    x: frame.cellW * g[0] + frame.gap / 2 + frame.offX
    y: frame.cellH * g[1] + frame.gap / 2 + frame.offY
    width: Math.max(24, frame.cellW * g[2] - frame.gap + frame.offW)
    height: Math.max(24, frame.cellH * g[3] - frame.gap + frame.offH)

    function commitMove() {
        const dx = Math.round(frame.offX / frame.cellW)
        const dy = Math.round(frame.offY / frame.cellH)
        frame.offX = 0
        frame.offY = 0
        if (dx !== 0 || dy !== 0)
            Layouts.setTile(frame.tileId, g[0] + dx, g[1] + dy, g[2], g[3])
    }

    function commitResize() {
        const dw = Math.round(frame.offW / frame.cellW)
        const dh = Math.round(frame.offH / frame.cellH)
        frame.offW = 0
        frame.offH = 0
        if (dw !== 0 || dh !== 0)
            Layouts.setTile(frame.tileId, g[0], g[1], g[2] + dw, g[3] + dh)
    }

    // Contenu de la zone. clip : une tuile rétrécie rogne son contenu au lieu de
    // le laisser déborder sur les voisines.
    Item {
        id: body
        anchors.fill: parent
        clip: true
    }

    // ---- Habillage du mode édition (au-dessus du contenu) ----
    Rectangle {
        anchors.fill: parent
        visible: Layouts.editMode
        color: frame.busy ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.12)
                          : Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.04)
        border.width: 1
        border.color: frame.busy ? Theme.accent : Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.45)
    }

    MouseArea {
        id: moveArea
        anchors.fill: parent
        enabled: Layouts.editMode
        visible: Layouts.editMode
        cursorShape: Qt.SizeAllCursor
        property real sx: 0
        property real sy: 0
        onPressed: (m) => { moveArea.sx = m.x; moveArea.sy = m.y }
        onPositionChanged: (m) => {
            frame.offX += m.x - moveArea.sx
            frame.offY += m.y - moveArea.sy
        }
        onReleased: frame.commitMove()
        onCanceled: { frame.offX = 0; frame.offY = 0 }
    }

    // Poignée de redimensionnement (coin bas-droit)
    Rectangle {
        width: 16; height: 16
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        visible: Layouts.editMode
        color: sizeArea.pressed ? Theme.accent
                                : Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.55)

        // Deux traits obliques : le repère universel « je m'étire ».
        Rectangle { x: 3; y: 11; width: 10; height: 1.5; color: Theme.bgBase; rotation: -45 }
        Rectangle { x: 6; y: 12; width: 6;  height: 1.5; color: Theme.bgBase; rotation: -45 }

        MouseArea {
            id: sizeArea
            anchors.fill: parent
            anchors.margins: -4          // cible plus facile à attraper
            cursorShape: Qt.SizeFDiagCursor
            property real sx: 0
            property real sy: 0
            onPressed: (m) => { sizeArea.sx = m.x; sizeArea.sy = m.y }
            onPositionChanged: (m) => {
                frame.offW += m.x - sizeArea.sx
                frame.offH += m.y - sizeArea.sy
            }
            onReleased: frame.commitResize()
            onCanceled: { frame.offW = 0; frame.offH = 0 }
        }
    }

    // Étiquette d'identification, utile quand une zone est réduite au point de
    // ne plus montrer son titre.
    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 3
        visible: Layouts.editMode
        text: frame.tileId.toUpperCase()
        color: Theme.accent
        font.family: Theme.fontUi; font.pixelSize: 9; font.letterSpacing: 1.5
        font.weight: Font.DemiBold
    }
}
