import QtQuick
import QtQuick.Window
import PerformanceOverlay

// Zone MÉTÉO : illustration animée + fond teinté selon les conditions.
// Le fond ne fait que TEINTER (alpha très bas) : il donne le caractère du temps
// qu'il fait sans jamais concurrencer la lecture des chiffres.
// Températures arrondies à l'entier — la décimale d'une température extérieure
// n'apporte rien et fait du bruit visuel.
Panel {
    id: root

    readonly property bool ok: Metrics.weather.valid

    // Même garde-fou que la sphère et l'icône de ping : rien ne s'anime quand la
    // fenêtre n'a pas le focus, ni en mode passif.
    readonly property bool animate: root.visible && root.ok
                                    && (Window.active || Config.animateInBackground)
                                    && !Overlay.clickThrough

    // Teinte par famille de temps : chaude au soleil, froide sous la pluie,
    // laiteuse dans la brume, sourde sous l'orage.
    readonly property color tint: {
        switch (icon.kind) {
        case "clear":  return Qt.rgba(Theme.warn.r, Theme.warn.g, Theme.warn.b, 0.10)
        case "partly": return Qt.rgba(Theme.warn.r, Theme.warn.g, Theme.warn.b, 0.06)
        case "rain":   return Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.09)
        case "snow":   return Qt.rgba(0.80, 0.90, 1.00, 0.08)
        case "storm":  return Qt.rgba(Theme.accent2.r, Theme.accent2.g, Theme.accent2.b, 0.09)
        case "fog":    return Qt.rgba(0.72, 0.78, 0.88, 0.07)
        default:       return Qt.rgba(1, 1, 1, 0.02)
        }
    }

    title: "MÉTÉO"
    tag: Metrics.weather.location
    accent: Theme.accent2
    statusColor: root.ok ? Theme.accent2 : Theme.muted

    // ---- Fond teinté, derrière le contenu ----
    // Il occupe la zone de contenu du panneau (pas le panneau entier) : sans
    // fondu aux DEUX extrémités, son bord haut se lisait comme une boîte posée
    // au milieu de la tuile. D'où les trois arrêts, transparents en haut et en bas.
    Rectangle {
        anchors.fill: parent
        z: -1
        opacity: root.ok ? 1 : 0
        gradient: Gradient {
            GradientStop { position: 0.0;  color: "transparent" }
            GradientStop { position: 0.45; color: root.tint }
            GradientStop { position: 1.0;  color: "transparent" }
        }
        Behavior on opacity { NumberAnimation { duration: 600 } }
    }

    Column {
        anchors.centerIn: parent
        width: parent.width
        spacing: Theme.px(3)

        // Illustration et température CÔTE À CÔTE : en tuile portrait, les
        // empiler faisait déborder le contenu hors du panneau.
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Theme.px(6)

            WeatherIcon {
                id: icon
                width: Theme.px(38); height: Theme.px(38)
                anchors.verticalCenter: parent.verticalCenter
                code: Metrics.weather.code
                animate: root.animate
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: root.ok ? Math.round(Metrics.weather.temperatureC) + "°" : "—"
                color: Theme.textHi
                font.family: Theme.fontMono; font.pixelSize: Theme.fsBig
                font.weight: Font.Black; font.features: ({ "tnum": 1 })
            }
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: root.ok ? Metrics.weather.description : "indispo."
            color: Theme.muted
            font.family: Theme.fontUi; font.pixelSize: Theme.fsMicro
            elide: Text.ElideRight
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.ok ? Math.round(Metrics.weather.maxC) + "° / " + Math.round(Metrics.weather.minC) + "°" : ""
            color: Theme.text
            font.family: Theme.fontMono; font.pixelSize: Theme.fsLabel; font.features: ({ "tnum": 1 })
        }
    }
}
