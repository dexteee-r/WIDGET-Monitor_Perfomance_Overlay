import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import PerformanceOverlay

// Routeur d'écrans : fond ambiant commun + header partagé (avec NavBar) + Loader
// qui charge la vue active (cockpit / tasks / settings). La vue « compact »
// occupe tout le cadre (sa propre chrome). Vue pilotée par le singleton Nav.
Item {
    id: shell

    component Chip: Column {
        property string k: ""
        property string v: ""
        spacing: 2
        Text { text: k; color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: Theme.fsLabel; font.letterSpacing: Theme.lsLabel }
        Text { text: v; color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: Theme.fsValue
               font.weight: Font.DemiBold; font.features: ({ "tnum": 1 }) }
    }

    // ================= FOND =================
    Rectangle { anchors.fill: parent; color: Theme.bgBase }
    Item {
        anchors.fill: parent
        Rectangle { id: orbA; width: 640; height: 640; radius: width / 2; x: -180; y: -240
                    color: Theme.accent; visible: false; layer.enabled: true }
        MultiEffect { source: orbA; anchors.fill: orbA; blurEnabled: true; blur: 1.0; blurMax: 64; opacity: 0.10 }
        Rectangle { id: orbB; width: 720; height: 720; radius: width / 2
                    x: parent.width - 500; y: parent.height - 480
                    color: Theme.accent2; visible: false; layer.enabled: true }
        MultiEffect { source: orbB; anchors.fill: orbB; blurEnabled: true; blur: 1.0; blurMax: 64; opacity: 0.09 }
    }
    Rectangle { anchors.fill: parent; color: "transparent"; border.width: 1; border.color: Theme.border }

    // ================= VUE COMPACTE (plein cadre) =================
    Loader {
        anchors.fill: parent
        active: Nav.view === "compact"
        sourceComponent: compactC
    }

    // ================= VUES STANDARD (header + écran) =================
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.gap
        spacing: Theme.gap
        visible: Nav.view !== "compact"

        // ---------- HEADER PARTAGÉ ----------
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: Theme.px(66)
            border.width: 1; border.color: Theme.border
            gradient: Gradient {
                GradientStop { position: 0.0; color: Theme.panelTop }
                GradientStop { position: 1.0; color: Theme.bgBase2 }
            }
            Rectangle { x: 0; y: 0; width: Theme.px(44); height: 2; color: Theme.accent }

            // Header en RowLayout : les 3 blocs (identité / navigation / statut)
            // se poussent au lieu de se chevaucher. Les espaceurs absorbent la
            // largeur restante ; le titre s'élide si le nom de machine est long.
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: Theme.px(22)
                anchors.rightMargin: Theme.px(22)
                spacing: 16

                Rectangle {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: Theme.px(10); Layout.preferredHeight: Theme.px(10)
                    radius: 5; color: Theme.ok
                }
                Text {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.maximumWidth: Theme.px(300)
                    text: (Metrics.hostName + " perf").toUpperCase(); color: Theme.textHi
                    font.family: Theme.fontUi; font.pixelSize: Theme.px(16); font.weight: Font.Bold
                    font.letterSpacing: Theme.lsTitle; elide: Text.ElideRight
                }

                Item { Layout.fillWidth: true; Layout.preferredWidth: 1 }

                NavBar { Layout.alignment: Qt.AlignVCenter }

                // Rien pour la disposition ici : organiser ses zones est un
                // RÉGLAGE, pas une commande permanente. Elle vit donc dans l'écran
                // Réglages ; pendant l'édition, une barre flottante (plus bas)
                // porte « Terminer » et « Réinitialiser ».

                Item { Layout.fillWidth: true; Layout.preferredWidth: 1 }

                Row {
                    Layout.alignment: Qt.AlignVCenter
                    spacing: 18
                    Row {
                        spacing: 9
                        anchors.verticalCenter: parent.verticalCenter
                        Rectangle { width: Theme.px(9); height: Theme.px(9); radius: width / 2
                                    color: Overlay.clickThrough ? Theme.warn : Theme.ok
                                    anchors.verticalCenter: parent.verticalCenter }
                        Column {
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 2
                            Text { text: Overlay.clickThrough ? "PASSIF" : "INTERACTIF"
                                   color: Theme.textHi; font.family: Theme.fontUi; font.pixelSize: Theme.fsBody; font.letterSpacing: Theme.lsLabel }
                            Text { text: "Ctrl+Alt+O"; color: Theme.muted; font.family: Theme.fontUi
                                   font.pixelSize: Theme.fsMicro; font.letterSpacing: Theme.lsLabel }
                        }
                    }
                    Rectangle { width: 1; height: Theme.px(40); color: Theme.border; anchors.verticalCenter: parent.verticalCenter }
                    // La puce PROCESSUS a été retirée : la zone SYSTÈME affiche déjà
                    // le compte, et le header n'avait plus la largeur pour tout tenir
                    // (l'horloge sortait de l'écran).
                    Chip { k: "UPTIME"; v: Fmt.uptime(Metrics.uptime.seconds); anchors.verticalCenter: parent.verticalCenter }
                    Rectangle { width: 1; height: Theme.px(40); color: Theme.border; anchors.verticalCenter: parent.verticalCenter }
                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2
                        Text { anchors.right: parent.right
                               text: Metrics.dateTime.now.toLocaleTimeString(Qt.locale("fr_FR"), "HH:mm:ss")
                               color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: Theme.px(28)
                               font.weight: Font.Bold; font.features: ({ "tnum": 1 }) }
                        Text { anchors.right: parent.right
                               text: Metrics.dateTime.now.toLocaleDateString(Qt.locale("fr_FR"), "ddd dd MMM").toUpperCase()
                               color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: Theme.fsLabel; font.letterSpacing: Theme.lsTitle }
                    }
                }
            }
        }

        // ---------- ÉCRAN ACTIF ----------
        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: Nav.view === "tasks" ? tasksC
                             : Nav.view === "settings" ? settingsC : cockpitC
        }
    }

    // ================= VOILE D'ALERTE (au-dessus de tout) =================
    // Seuils critiques : GPU très chaud, ou CPU/RAM quasi saturés.
    readonly property bool _gpuHot: Metrics.gpu.available && Metrics.gpu.temperatureC >= 88
    readonly property bool _cpuMax: Metrics.cpu.usagePercent >= 96
    readonly property bool _ramMax: Metrics.ram.usagePercent >= 95
    readonly property string _alertLabel:
        _gpuHot ? "ALERTE THERMIQUE GPU · " + Metrics.gpu.temperatureC.toFixed(0) + "°C"
        : _cpuMax ? "CHARGE CPU CRITIQUE · " + Metrics.cpu.usagePercent.toFixed(0) + "%"
        : _ramMax ? "MÉMOIRE SATURÉE · " + Metrics.ram.usagePercent.toFixed(0) + "%" : ""

    AlertVeil {
        anchors.fill: parent
        active: shell._gpuHot || shell._cpuMax || shell._ramMax
        label: shell._alertLabel
    }

    // ===== Barre flottante du mode « organiser les zones » =====
    // N'existe que pendant l'édition : le header reste donc propre en usage
    // normal, et on garde une sortie visible sans avoir à connaître F3.
    Rectangle {
        visible: Layouts.editMode
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.gap * 2
        width: editRow.implicitWidth + Theme.px(28)
        height: Theme.px(44)
        radius: Theme.radius
        color: Theme.panelBot
        border.width: 1
        border.color: Theme.accent

        component EditBtn: Rectangle {
            id: eb
            property string label: ""
            property bool primary: false
            signal clicked()
            implicitWidth: ebt.implicitWidth + Theme.px(20)
            implicitHeight: Theme.px(26)
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
            color: eb.primary ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.18) : "transparent"
            border.width: 1
            border.color: eb.primary ? Theme.accent : Theme.border
            Text {
                id: ebt
                anchors.centerIn: parent
                text: eb.label
                color: eb.primary ? Theme.accent : Theme.muted
                font.family: Theme.fontUi; font.pixelSize: Theme.fsLabel
                font.letterSpacing: Theme.lsLabel
                font.weight: eb.primary ? Font.DemiBold : Font.Normal
            }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: eb.clicked()
            }
        }

        Row {
            id: editRow
            anchors.centerIn: parent
            spacing: Theme.px(10)
            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: "ORGANISER LES ZONES"
                color: Theme.textHi
                font.family: Theme.fontUi; font.pixelSize: Theme.fsLabel
                font.letterSpacing: Theme.lsTitle; font.weight: Font.DemiBold
            }
            EditBtn { label: "TERMINER"; primary: true; onClicked: Layouts.editMode = false }
            EditBtn { label: "RÉINITIALISER"; onClicked: Layouts.reset() }
        }
    }

    Component { id: cockpitC;  CockpitScreen {} }
    Component { id: tasksC;    TasksScreen {} }
    Component { id: settingsC; SettingsScreen {} }
    Component { id: compactC;  CompactScreen {} }
}
