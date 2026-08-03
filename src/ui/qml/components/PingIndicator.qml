import QtQuick
import QtQuick.Shapes
import QtQuick.Window
import PerformanceOverlay

// Indicateur de ping — bloc autonome (pas un panneau) destiné à vivre DANS la
// zone RÉSEAU : la latence est une métrique réseau, elle appartient au même
// panneau que le débit.
//
// L'icône « émet » en boucle (arcs qui s'allument du centre vers l'extérieur)
// tant que l'hôte répond ; elle se fige et rougit dès que le ping échoue —
// l'animation porte donc l'information, pas seulement le chiffre.
//
// ⚠️ PERF : trois animations d'opacité déclaratives (aucun JS par frame),
// arrêtées dès que la fenêtre perd le focus, comme la sphère.
Item {
    id: root

    readonly property int ms: Metrics.ping.latencyMs
    readonly property bool up: Metrics.ping.reachable
    readonly property color lat: !up ? Theme.crit
                                : ms >= 120 ? Theme.crit
                                : ms >= 60 ? Theme.warn : Theme.ok
    readonly property bool animate: root.visible && root.up
                                    && (Window.active || Config.animateInBackground)
                                    && !Overlay.clickThrough

    implicitWidth: Theme.px(150)
    implicitHeight: Theme.px(96)

    Column {
        anchors.centerIn: parent
        spacing: Theme.px(5)

        Item {
            id: icon
            width: Theme.px(78); height: Theme.px(42)
            anchors.horizontalCenter: parent.horizontalCenter

            Repeater {
                model: 3

                Shape {
                    id: arc
                    required property int index
                    anchors.fill: parent
                    preferredRendererType: Shape.CurveRenderer
                    opacity: root.up ? 0.22 : 0.12

                    ShapePath {
                        strokeColor: root.lat
                        strokeWidth: Math.max(2, Theme.px(3))
                        fillColor: "transparent"
                        capStyle: ShapePath.RoundCap
                        PathAngleArc {
                            centerX: icon.width / 2
                            centerY: icon.height - 3
                            radiusX: Theme.px(11) + arc.index * Theme.px(13)
                            radiusY: Theme.px(11) + arc.index * Theme.px(13)
                            startAngle: 200
                            sweepAngle: 140
                        }
                    }

                    // Vague : chaque arc s'allume à son tour, du plus petit au plus
                    // grand. Le décalage vient du PauseAnimation, pas d'un timer JS.
                    SequentialAnimation on opacity {
                        running: root.animate
                        loops: Animation.Infinite
                        PauseAnimation { duration: arc.index * 200 }
                        NumberAnimation { to: 1.0; duration: 320; easing.type: Easing.OutCubic }
                        NumberAnimation { to: 0.22; duration: 700; easing.type: Easing.InOutQuad }
                        PauseAnimation { duration: (2 - arc.index) * 200 }
                    }
                }
            }

            Rectangle {
                width: 5; height: 5; radius: 2.5
                color: root.lat
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 3
            Text {
                id: msText
                text: root.up ? "" + root.ms : "—"
                color: root.lat
                font.family: Theme.fontMono; font.pixelSize: Theme.fsBig
                font.weight: Font.Black; font.features: ({ "tnum": 1 })
            }
            Text {
                text: "ms"; visible: root.up
                color: Theme.muted; font.family: Theme.fontMono; font.pixelSize: Theme.fsLabel
                anchors.bottom: msText.bottom; anchors.bottomMargin: 3
            }
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Metrics.ping.lossPercent > 0
                  ? "perte " + Metrics.ping.lossPercent.toFixed(0) + " %"
                  : Metrics.ping.host
            color: Metrics.ping.lossPercent >= 5 ? Theme.crit : Theme.muted
            font.family: Theme.fontUi; font.pixelSize: Theme.fsMicro; font.letterSpacing: 0.8
        }
    }
}
