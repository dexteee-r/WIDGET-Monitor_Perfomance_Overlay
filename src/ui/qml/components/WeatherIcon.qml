import QtQuick
import QtQuick.Shapes
import PerformanceOverlay

// Illustration météo animée, dessinée en primitives QML (aucune image).
// La famille de temps vient du code WMO d'Open-Meteo ; chaque famille a son
// mouvement propre : le soleil tourne et respire, les nuages dérivent, la pluie
// tombe, la neige descend en zigzag, l'éclair claque, la brume ondule.
//
// ⚠️ PERF : uniquement des animations déclaratives (aucun JS par frame), et
// TOUTES arrêtées quand la fenêtre perd le focus — même garde-fou que la sphère
// et l'icône de ping. Au repos, la scène ne se redessine pas.
Item {
    id: root

    property int code: -1
    property bool animate: true

    // Familles WMO → un seul endroit où la correspondance est décidée.
    readonly property string kind: {
        const c = root.code
        if (c < 0) return "none"
        if (c === 0) return "clear"
        if (c === 1 || c === 2) return "partly"
        if (c === 3) return "cloudy"
        if (c >= 45 && c <= 48) return "fog"
        if (c >= 71 && c <= 77) return "snow"
        if (c === 85 || c === 86) return "snow"
        if (c >= 95) return "storm"
        if (c >= 51 && c <= 67) return "rain"
        if (c >= 80 && c <= 82) return "rain"
        return "cloudy"
    }

    readonly property bool hasSun: kind === "clear" || kind === "partly"
    readonly property bool hasCloud: kind !== "clear" && kind !== "fog" && kind !== "none"
    readonly property bool hasDrops: kind === "rain"
    readonly property bool hasFlakes: kind === "snow"

    readonly property real u: Math.min(width, height) / 100   // unité relative

    // ---------------- Soleil ----------------
    Item {
        id: sun
        width: root.u * 46; height: width
        x: root.kind === "clear" ? (root.width - width) / 2 : root.width * 0.16
        y: root.kind === "clear" ? (root.height - height) / 2 : root.height * 0.10
        visible: root.hasSun

        // Halo qui respire
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 1.5; height: width; radius: width / 2
            color: Qt.rgba(Theme.warn.r, Theme.warn.g, Theme.warn.b, 0.13)
            SequentialAnimation on scale {
                running: root.animate && sun.visible
                loops: Animation.Infinite
                NumberAnimation { to: 1.12; duration: 2200; easing.type: Easing.InOutSine }
                NumberAnimation { to: 1.00; duration: 2200; easing.type: Easing.InOutSine }
            }
        }

        // Rayons (RotationAnimator = animateur natif, pas de JS)
        Item {
            id: rays
            anchors.fill: parent
            Repeater {
                model: 8
                Rectangle {
                    required property int index
                    width: root.u * 3; height: root.u * 8
                    radius: width / 2
                    color: Theme.warn
                    opacity: 0.75
                    x: (rays.width - width) / 2
                    y: -root.u * 3
                    transform: Rotation {
                        origin.x: width / 2
                        origin.y: rays.height / 2 + root.u * 3
                        angle: index * 45
                    }
                }
            }
            RotationAnimator on rotation {
                running: root.animate && sun.visible
                from: 0; to: 360; duration: 24000
                loops: Animation.Infinite
            }
        }

        // Disque
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 0.62; height: width; radius: width / 2
            color: Theme.warn
        }
    }

    // ---------------- Nuage ----------------
    Item {
        id: cloud
        width: root.u * 62; height: root.u * 30
        y: root.height * (root.hasDrops || root.hasFlakes ? 0.24 : 0.34)
        visible: root.hasCloud

        // Décalage = position de base (décalée en « partiellement nuageux »
        // pour dégager le soleil) + dérive animée. On anime `drift` et non
        // l'offset lui-même, sinon l'animation écraserait la position de base.
        property real drift: 0
        anchors.horizontalCenter: root.horizontalCenter
        anchors.horizontalCenterOffset: (root.kind === "partly" ? root.u * 8 : 0) + drift

        readonly property color body: root.kind === "storm"
                                      ? Qt.rgba(0.62, 0.66, 0.78, 1.0)
                                      : Qt.rgba(0.78, 0.83, 0.92, 1.0)

        Rectangle { x: 0; y: parent.height * 0.42; width: parent.width; height: parent.height * 0.58
                    radius: height / 2; color: cloud.body }
        Rectangle { x: parent.width * 0.14; y: 0; width: parent.width * 0.42; height: parent.width * 0.42
                    radius: width / 2; color: cloud.body }
        Rectangle { x: parent.width * 0.46; y: parent.height * 0.14; width: parent.width * 0.34
                    height: width; radius: width / 2; color: cloud.body }

        // Dérive lente
        SequentialAnimation on drift {
            running: root.animate && cloud.visible
            loops: Animation.Infinite
            NumberAnimation { to: root.u * 3;  duration: 4000; easing.type: Easing.InOutSine }
            NumberAnimation { to: -root.u * 3; duration: 4000; easing.type: Easing.InOutSine }
        }
    }

    // ---------------- Pluie ----------------
    Repeater {
        model: root.hasDrops ? 4 : 0
        Rectangle {
            id: drop
            required property int index
            width: root.u * 2.4; height: root.u * 9
            radius: width / 2
            color: Theme.accent
            opacity: 0.85
            x: root.width * 0.30 + drop.index * root.u * 13
            y: root.height * 0.52

            SequentialAnimation {
                running: root.animate
                loops: Animation.Infinite
                PauseAnimation { duration: drop.index * 190 }
                ParallelAnimation {
                    NumberAnimation { target: drop; property: "y"
                                      from: root.height * 0.52; to: root.height * 0.86
                                      duration: 750; easing.type: Easing.InQuad }
                    NumberAnimation { target: drop; property: "opacity"
                                      from: 0.9; to: 0.0; duration: 750 }
                }
                PauseAnimation { duration: (3 - drop.index) * 190 }
            }
        }
    }

    // ---------------- Neige ----------------
    Repeater {
        model: root.hasFlakes ? 4 : 0
        Rectangle {
            id: flake
            required property int index
            readonly property real baseX: root.width * 0.28 + flake.index * root.u * 13
            width: root.u * 5; height: width; radius: width / 2
            color: "#FFFFFF"
            opacity: 0.85
            x: flake.baseX
            y: root.height * 0.52

            SequentialAnimation {
                running: root.animate
                loops: Animation.Infinite
                PauseAnimation { duration: flake.index * 300 }
                ParallelAnimation {
                    NumberAnimation { target: flake; property: "y"
                                      from: root.height * 0.52; to: root.height * 0.88
                                      duration: 2100; easing.type: Easing.InOutSine }
                    NumberAnimation { target: flake; property: "opacity"
                                      from: 0.9; to: 0.0; duration: 2100 }
                    // Zigzag : c'est ce qui distingue la neige de la pluie.
                    SequentialAnimation {
                        NumberAnimation { target: flake; property: "x"
                                          to: flake.baseX + root.u * 4
                                          duration: 1050; easing.type: Easing.InOutSine }
                        NumberAnimation { target: flake; property: "x"
                                          to: flake.baseX
                                          duration: 1050; easing.type: Easing.InOutSine }
                    }
                }
            }
        }
    }

    // ---------------- Éclair ----------------
    Shape {
        id: bolt
        visible: root.kind === "storm"
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer
        opacity: 0

        ShapePath {
            fillColor: Theme.warn
            strokeColor: "transparent"
            startX: root.width * 0.50; startY: root.height * 0.50
            PathLine { x: root.width * 0.42; y: root.height * 0.74 }
            PathLine { x: root.width * 0.50; y: root.height * 0.74 }
            PathLine { x: root.width * 0.44; y: root.height * 0.94 }
            PathLine { x: root.width * 0.62; y: root.height * 0.66 }
            PathLine { x: root.width * 0.53; y: root.height * 0.66 }
            PathLine { x: root.width * 0.60; y: root.height * 0.50 }
        }

        SequentialAnimation on opacity {
            running: root.animate && bolt.visible
            loops: Animation.Infinite
            PauseAnimation { duration: 2400 }
            NumberAnimation { to: 1.0; duration: 60 }
            NumberAnimation { to: 0.2; duration: 90 }
            NumberAnimation { to: 1.0; duration: 60 }
            NumberAnimation { to: 0.0; duration: 320 }
        }
    }

    // ---------------- Brume ----------------
    Repeater {
        model: root.kind === "fog" ? 3 : 0
        Rectangle {
            required property int index
            width: root.width * 0.56; height: root.u * 4
            radius: height / 2
            color: Qt.rgba(0.78, 0.83, 0.92, 0.55)
            x: root.width * 0.22
            y: root.height * (0.34 + index * 0.16)

            SequentialAnimation on x {
                running: root.animate
                loops: Animation.Infinite
                PauseAnimation { duration: index * 400 }
                NumberAnimation { to: root.width * 0.30; duration: 2600; easing.type: Easing.InOutSine }
                NumberAnimation { to: root.width * 0.14; duration: 2600; easing.type: Easing.InOutSine }
                NumberAnimation { to: root.width * 0.22; duration: 1300; easing.type: Easing.InOutSine }
            }
        }
    }
}
