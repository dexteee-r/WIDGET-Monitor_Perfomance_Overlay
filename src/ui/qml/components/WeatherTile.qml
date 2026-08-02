import QtQuick
import PerformanceOverlay

// Zone MÉTÉO — tuile verticale (portrait), pensée pour tenir à côté de PRIÈRE.
// Températures arrondies à l'entier : la décimale d'une température extérieure
// n'apporte rien et fait du bruit visuel.
Panel {
    id: root

    readonly property bool ok: Metrics.weather.valid

    title: "MÉTÉO"
    accent: Theme.accent2
    statusColor: root.ok ? Theme.accent2 : Theme.muted

    Column {
        anchors.centerIn: parent
        width: parent.width
        spacing: 2

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Metrics.weather.symbol
            color: Theme.accent2
            font.family: Theme.fontUi; font.pixelSize: 20
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.ok ? Math.round(Metrics.weather.temperatureC) + "°" : "—"
            color: Theme.textHi
            font.family: Theme.fontMono; font.pixelSize: 28
            font.weight: Font.Black; font.features: ({ "tnum": 1 })
        }
        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: root.ok ? Metrics.weather.description : "indispo."
            color: Theme.muted
            font.family: Theme.fontUi; font.pixelSize: 9; font.letterSpacing: 0.4
            elide: Text.ElideRight
        }
        Item { width: 1; height: 4 }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.ok ? Math.round(Metrics.weather.maxC) + "° / " + Math.round(Metrics.weather.minC) + "°" : ""
            color: Theme.text
            font.family: Theme.fontMono; font.pixelSize: 11; font.features: ({ "tnum": 1 })
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: Metrics.weather.location
            color: Theme.muted
            font.family: Theme.fontUi; font.pixelSize: 8; font.letterSpacing: 1
        }
    }
}
