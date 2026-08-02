import QtQuick
import QtQuick.Layouts
import PerformanceOverlay

// Zone CHRONO / MINUTEUR — petite tuile carrée.
// Le temps se calcule sur l'horloge murale (deltas de Date.now()) et non en
// additionnant les intervalles du Timer : un tick en retard (charge GPU, fenêtre
// masquée) ne fait donc PAS dériver le compte.
// Le ticker ne tourne que pendant une mesure en cours — au repos, coût nul.
Panel {
    id: root

    property bool countdown: false          // false = chrono, true = minuteur
    property bool running: false
    property double accumMs: 0
    property double lastTick: 0
    property int presetMin: 5
    property bool ended: false              // minuteur arrivé à zéro

    readonly property double shownMs: countdown
                                      ? Math.max(0, presetMin * 60000 - accumMs)
                                      : accumMs

    title: countdown ? "MINUTEUR" : "CHRONO"
    tag: countdown ? presetMin + " MIN" : ""
    accent: root.ended ? Theme.crit : Theme.accent
    statusColor: root.ended ? Theme.crit : (root.running ? Theme.ok : Theme.muted)
    alert: root.ended

    function fmt(ms) {
        const total = Math.floor(ms / 1000)
        const h = Math.floor(total / 3600)
        const m = Math.floor((total % 3600) / 60)
        const s = total % 60
        const p2 = (v) => (v < 10 ? "0" + v : "" + v)
        if (h > 0)
            return h + ":" + p2(m) + ":" + p2(s)
        return p2(m) + ":" + p2(s)
    }

    function start() {
        if (root.countdown && root.shownMs <= 0)
            root.reset()
        root.ended = false
        root.lastTick = Date.now()
        root.running = true
    }
    function reset() {
        root.running = false
        root.accumMs = 0
        root.ended = false
    }

    Timer {
        interval: 200
        repeat: true
        running: root.running
        onTriggered: {
            const now = Date.now()
            root.accumMs += now - root.lastTick
            root.lastTick = now
            if (root.countdown && root.accumMs >= root.presetMin * 60000) {
                root.accumMs = root.presetMin * 60000
                root.running = false
                root.ended = true
            }
        }
    }

    component Btn: Rectangle {
        id: btn
        property string label: ""
        property bool on: false
        signal clicked()
        implicitWidth: t.implicitWidth + 12
        implicitHeight: 20
        color: on ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16) : "transparent"
        border.width: 1
        border.color: on ? Theme.accent : Theme.border
        Text {
            id: t
            anchors.centerIn: parent
            text: btn.label
            color: btn.on ? Theme.accent : Theme.muted
            font.family: Theme.fontUi; font.pixelSize: 9; font.letterSpacing: 1
            font.weight: btn.on ? Font.DemiBold : Font.Normal
        }
        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: btn.clicked()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 5

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 4
            Btn { label: "CHRONO"; on: !root.countdown
                  onClicked: { root.countdown = false; root.reset() } }
            Btn { label: "MINUT."; on: root.countdown
                  onClicked: { root.countdown = true; root.reset() } }
        }

        Text {
            Layout.alignment: Qt.AlignHCenter
            text: root.fmt(root.shownMs)
            color: root.ended ? Theme.crit : Theme.textHi
            font.family: Theme.fontMono; font.pixelSize: 26
            font.weight: Font.Black; font.features: ({ "tnum": 1 })
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 4
            Btn {
                label: root.running ? "PAUSE" : "GO"
                on: root.running
                onClicked: root.running ? (root.running = false) : root.start()
            }
            Btn { label: "RAZ"; onClicked: root.reset() }
            // Durée du minuteur : un seul bouton qui fait défiler 5 → 10 → 25.
            Btn {
                visible: root.countdown
                label: root.presetMin + "M"
                onClicked: {
                    root.presetMin = root.presetMin === 5 ? 10
                                   : root.presetMin === 10 ? 25 : 5
                    root.reset()
                }
            }
        }

        Item { Layout.fillHeight: true }
    }
}
