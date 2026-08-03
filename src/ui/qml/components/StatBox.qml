import QtQuick
import PerformanceOverlay

// Petite boîte de stat encadrée : label + grosse valeur (chiffres tabulaires) +
// unité. Couleur de valeur surchargeable pour les états (warn/crit).
Item {
    id: s

    property string label: ""
    property string value: ""
    property string unit: ""
    property color valueColor: Theme.textHi

    implicitWidth: Theme.px(92)
    implicitHeight: Theme.px(54)

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.25)
        border.width: 1
        border.color: Theme.border2
    }

    Column {
        anchors.centerIn: parent
        spacing: Theme.px(4)

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: s.label
            color: Theme.muted
            font.family: Theme.fontUi
            font.pixelSize: Theme.fsLabel
            font.letterSpacing: Theme.lsLabel
        }
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Theme.px(3)
            Text {
                text: s.value
                color: s.valueColor
                font.family: Theme.fontMono
                font.pixelSize: Theme.fsLead
                font.weight: Font.Bold
                font.features: ({ "tnum": 1 })
            }
            Text {
                anchors.baseline: parent.children[0].baseline
                text: s.unit
                visible: s.unit !== ""
                color: Theme.muted
                font.family: Theme.fontMono
                font.pixelSize: Theme.fsBody
            }
        }
    }
}
