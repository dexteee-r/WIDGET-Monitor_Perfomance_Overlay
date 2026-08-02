import QtQuick
import QtQuick.Layouts
import PerformanceOverlay

// Vue cockpit — grille libre. Le fond et le header sont fournis par AppShell.
//
// Les zones ne sont plus empilées dans des layouts imbriqués mais posées sur une
// grille de 24 × 16 cellules : c'est ce qui rend le déplacement et le
// redimensionnement possibles (on ne déplace pas un item dans un ColumnLayout).
// Chaque zone lit sa géométrie dans le singleton `Layouts`, qui la persiste dans
// config.ini. La grille étant en unités relatives, la MÊME disposition sert au
// cockpit (1280×720) et au plein écran (2560×1440) — même rapport 16/9.
Item {
    id: cockpit

    component KV: Item {
        property string k: ""
        property string v: ""
        property color vc: Theme.textHi
        implicitHeight: 30
        Text { anchors.left: parent.left; anchors.verticalCenter: parent.verticalCenter
               text: k; color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 12; font.letterSpacing: 1 }
        Text { anchors.right: parent.right; anchors.verticalCenter: parent.verticalCenter
               text: v; color: vc; font.family: Theme.fontMono; font.pixelSize: 14
               font.weight: Font.DemiBold; font.features: ({ "tnum": 1 }) }
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.border2 }
    }

    // Ligne de mémoire compacte : libellé · détail · % + barre segmentée.
    component MemRow: ColumnLayout {
        id: mrow
        property string label: ""
        property real ratio: 0        // 0..1
        property string detail: ""
        spacing: 5
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Text { text: mrow.label; color: Theme.muted
                   font.family: Theme.fontUi; font.pixelSize: 11; font.letterSpacing: 1.4 }
            Item { Layout.fillWidth: true }
            Text { text: mrow.detail; color: Theme.text
                   font.family: Theme.fontMono; font.pixelSize: 11; font.features: ({ "tnum": 1 }) }
            Text { text: Math.round(mrow.ratio * 100) + " %"; color: Theme.textHi
                   font.family: Theme.fontMono; font.pixelSize: 13
                   font.weight: Font.Bold; font.features: ({ "tnum": 1 }) }
        }
        SegmentBar { Layout.fillWidth: true; segments: 20; value: mrow.ratio }
    }

    component Readout: Column {
        property string big: ""
        property string unit: "%"
        property string sub: ""
        property color bigColor: Theme.textHi
        property int bigSize: 44
        spacing: 2
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 2
            Text { id: bigT; text: big; color: bigColor; font.family: Theme.fontMono
                   font.pixelSize: bigSize; font.weight: Font.Black; font.features: ({ "tnum": 1 }) }
            Text { text: unit; visible: unit !== ""; color: Theme.muted; font.family: Theme.fontMono
                   font.pixelSize: Math.round(bigSize * 0.36)
                   anchors.bottom: bigT.bottom; anchors.bottomMargin: Math.round(bigSize * 0.14) }
        }
        // Le sous-libellé disparaît sous une certaine taille de jauge : en dessous
        // il chevauchait le chiffre et mordait sur l'anneau au lieu d'informer.
        Text { anchors.horizontalCenter: parent.horizontalCenter; text: sub
               visible: sub !== "" && bigSize >= 26
               color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 10; font.letterSpacing: 3 }
    }

    // ======================= GRILLE LIBRE =======================
    Item {
        id: canvas
        anchors.fill: parent

        readonly property real cellW: width / Layouts.columns
        readonly property real cellH: height / Layouts.rows

        // Repères de grille, visibles seulement pendant l'édition.
        Repeater {
            model: Layouts.columns + 1
            Rectangle {
                required property int index
                visible: Layouts.editMode
                x: Math.round(index * canvas.cellW)
                width: 1; height: canvas.height
                color: Qt.rgba(1, 1, 1, 0.05)
            }
        }
        Repeater {
            model: Layouts.rows + 1
            Rectangle {
                required property int index
                visible: Layouts.editMode
                y: Math.round(index * canvas.cellH)
                height: 1; width: canvas.width
                color: Qt.rgba(1, 1, 1, 0.05)
            }
        }

        // ----------------------------- CPU -----------------------------
        TileFrame {
            tileId: "cpu"; cellW: canvas.cellW; cellH: canvas.cellH
            Panel {
                anchors.fill: parent
                title: "CPU"
                tag: Metrics.cpu.frequencyGhz.toFixed(2) + " GHz"
                statusColor: Theme.statusColor(Metrics.cpu.usagePercent / 100)
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Text {
                        Layout.fillWidth: true
                        text: Metrics.cpu.name; visible: text !== ""
                        color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 10
                        font.letterSpacing: 1; elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Item {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        CircularGauge {
                            id: cpuGauge
                            anchors.centerIn: parent
                            width: Math.min(parent.width, parent.height); height: width
                            lineWidth: 11
                            glow: true
                            value: Metrics.cpu.usagePercent / 100
                            arcColor: Theme.statusColor(Metrics.cpu.usagePercent / 100)
                            Readout {
                                anchors.centerIn: parent
                                bigSize: cpuGauge.readoutSize
                                big: Metrics.cpu.usagePercent.toFixed(0); sub: "CHARGE"
                                bigColor: Theme.statusColor(Metrics.cpu.usagePercent / 100)
                            }
                        }
                    }
                    Sparkline {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 30
                        values: Metrics.cpu.usageHistory
                        lineColor: Theme.statusColor(Metrics.cpu.usagePercent / 100)
                    }
                }
            }
        }

        // ----------------------------- GPU -----------------------------
        TileFrame {
            tileId: "gpu"; cellW: canvas.cellW; cellH: canvas.cellH
            Panel {
                anchors.fill: parent
                title: "GPU"
                tag: Metrics.gpu.available ? "NVML" : ""
                alert: Metrics.gpu.available && Metrics.gpu.temperatureC >= 85
                statusColor: !Metrics.gpu.available ? Theme.muted
                             : Metrics.gpu.temperatureC >= 85 ? Theme.crit
                             : Metrics.gpu.temperatureC >= 72 ? Theme.warn : Theme.ok
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Text {
                        Layout.fillWidth: true
                        text: Metrics.gpu.name; visible: Metrics.gpu.available && text !== ""
                        color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 10
                        font.letterSpacing: 1; elide: Text.ElideRight
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Item {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        CircularGauge {
                            id: gpuGauge
                            anchors.centerIn: parent
                            visible: Metrics.gpu.available
                            width: Math.min(parent.width, parent.height); height: width
                            lineWidth: 11
                            glow: true
                            value: Metrics.gpu.usagePercent / 100
                            arcColor: Metrics.gpu.temperatureC >= 85 ? Theme.crit
                                      : Metrics.gpu.temperatureC >= 72 ? Theme.warn : Theme.accent
                            Readout { anchors.centerIn: parent; bigSize: gpuGauge.readoutSize
                                      big: Metrics.gpu.usagePercent.toFixed(0); sub: "GPU" }
                        }
                        Text {
                            anchors.centerIn: parent; visible: !Metrics.gpu.available
                            text: Metrics.gpu.name; width: parent.width * 0.9
                            horizontalAlignment: Text.AlignHCenter; wrapMode: Text.WordWrap
                            color: Theme.text; font.family: Theme.fontUi; font.pixelSize: 15
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 8; visible: Metrics.gpu.available
                        StatBox {
                            Layout.fillWidth: true; label: "TEMP"
                            value: Metrics.gpu.temperatureC.toFixed(0); unit: "°C"
                            valueColor: Metrics.gpu.temperatureC >= 85 ? Theme.crit
                                        : Metrics.gpu.temperatureC >= 72 ? Theme.warn : Theme.textHi
                        }
                        StatBox { Layout.fillWidth: true; label: "CONSO"; value: Metrics.gpu.powerW.toFixed(0); unit: "W" }
                        StatBox { Layout.fillWidth: true; label: "VRAM"; value: Metrics.gpu.vramUsedGb.toFixed(1); unit: "GB" }
                    }
                }
            }
        }

        // --------------------------- MÉMOIRE ---------------------------
        TileFrame {
            tileId: "memory"; cellW: canvas.cellW; cellH: canvas.cellH
            Panel {
                anchors.fill: parent
                title: "MÉMOIRE"
                statusColor: Theme.statusColor(Metrics.ram.usagePercent / 100)
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12
                    MemRow {
                        Layout.fillWidth: true
                        label: "RAM"
                        ratio: Metrics.ram.usagePercent / 100
                        detail: Metrics.ram.usedGb.toFixed(1) + "/" + Metrics.ram.totalGb.toFixed(0) + "G"
                    }
                    MemRow {
                        Layout.fillWidth: true
                        visible: Metrics.gpu.available
                        label: "VRAM"
                        ratio: Metrics.gpu.vramTotalGb > 0
                               ? Metrics.gpu.vramUsedGb / Metrics.gpu.vramTotalGb : 0
                        detail: Metrics.gpu.vramUsedGb.toFixed(1) + "/" + Metrics.gpu.vramTotalGb.toFixed(0) + "G"
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }

        // ---------------------------- CHRONO ---------------------------
        TileFrame {
            tileId: "timer"; cellW: canvas.cellW; cellH: canvas.cellH
            TimerTile { anchors.fill: parent }
        }

        // ------------------------ FLUX DE CHARGE ------------------------
        TileFrame {
            tileId: "flux"; cellW: canvas.cellW; cellH: canvas.cellH
            Panel {
                anchors.fill: parent
                title: "FLUX DE CHARGE"
                statusColor: Theme.statusColor(Metrics.systemLoad)
                tag: "CPU · GPU"
                CenterVisual {
                    anchors.fill: parent
                    load: Metrics.systemLoad
                }
            }
        }

        // --------------------------- STOCKAGE ---------------------------
        TileFrame {
            tileId: "storage"; cellW: canvas.cellW; cellH: canvas.cellH
            Panel {
                anchors.fill: parent
                title: "STOCKAGE"
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10
                    Repeater {
                        model: Metrics.disk.disks
                        ColumnLayout {
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: 6
                            RowLayout {
                                Layout.fillWidth: true
                                Text { text: "Disque " + modelData.name; color: Theme.text
                                       font.family: Theme.fontUi; font.pixelSize: 12; font.letterSpacing: 1 }
                                Item { Layout.fillWidth: true }
                                Text { text: Math.round(modelData.usagePercent) + " %"; color: Theme.textHi
                                       font.family: Theme.fontMono; font.pixelSize: 13; font.weight: Font.Bold
                                       font.features: ({ "tnum": 1 }) }
                            }
                            SegmentBar { Layout.fillWidth: true; segments: 34; value: modelData.usagePercent / 100 }
                        }
                    }
                    Item { Layout.fillHeight: true }
                }
            }
        }

        // ---------------------------- RÉSEAU ----------------------------
        TileFrame {
            tileId: "network"; cellW: canvas.cellW; cellH: canvas.cellH
            Panel {
                anchors.fill: parent
                title: "RÉSEAU"
                tag: Metrics.network.ipAddress
                RowLayout {
                    anchors.fill: parent
                    spacing: 14
                    ColumnLayout {
                        Layout.preferredWidth: 150
                        spacing: 8
                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "↓"; color: Theme.accent; font.family: Theme.fontMono; font.pixelSize: 16 }
                            Text { text: Fmt.rate(Metrics.network.downBytesPerSec); color: Theme.textHi
                                   font.family: Theme.fontMono; font.pixelSize: 15; font.weight: Font.DemiBold
                                   font.features: ({ "tnum": 1 }) }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: "↑"; color: Theme.accent2; font.family: Theme.fontMono; font.pixelSize: 16 }
                            Text { text: Fmt.rate(Metrics.network.upBytesPerSec); color: Theme.textHi
                                   font.family: Theme.fontMono; font.pixelSize: 15; font.weight: Font.DemiBold
                                   font.features: ({ "tnum": 1 }) }
                        }
                    }
                    // Graphe en miroir : descendant au-dessus de l'axe, montant en
                    // dessous, à la même échelle.
                    NetGraph {
                        Layout.fillWidth: true; Layout.fillHeight: true
                        down: Metrics.network.downHistory
                        up: Metrics.network.upHistory
                    }
                    // La latence est une métrique réseau : même panneau que le débit.
                    Rectangle {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 1
                        color: Theme.border2
                    }
                    PingIndicator {
                        Layout.preferredWidth: 92
                        Layout.fillHeight: true
                    }
                }
            }
        }

        // ---------------------------- PRIÈRE ----------------------------
        TileFrame {
            tileId: "prayer"; cellW: canvas.cellW; cellH: canvas.cellH
            Panel {
                anchors.fill: parent
                title: "PRIÈRE"
                accent: Theme.accent2
                statusColor: Theme.accent2
                tag: Metrics.prayer.usingApi ? "API" : ""
                Column {
                    anchors.centerIn: parent
                    spacing: 3
                    Text { anchors.horizontalCenter: parent.horizontalCenter
                           text: Metrics.prayer.nextName; color: Theme.accent2
                           font.family: Theme.fontUi; font.pixelSize: 24; font.weight: Font.Black }
                    Text { anchors.horizontalCenter: parent.horizontalCenter
                           text: Fmt.pad2(Metrics.prayer.nextHour) + ":" + Fmt.pad2(Metrics.prayer.nextMinute)
                           color: Theme.muted; font.family: Theme.fontMono; font.pixelSize: 11; font.features: ({ "tnum": 1 }) }
                    Text { anchors.horizontalCenter: parent.horizontalCenter
                           text: "dans " + Fmt.countdown(Metrics.prayer.remainingMinutes)
                           color: Theme.textHi; font.family: Theme.fontMono; font.pixelSize: 13
                           font.weight: Font.Bold; font.features: ({ "tnum": 1 }) }
                }
            }
        }

        // ---------------------------- MÉTÉO -----------------------------
        TileFrame {
            tileId: "weather"; cellW: canvas.cellW; cellH: canvas.cellH
            WeatherTile { anchors.fill: parent }
        }

        // -------------------- SYSTÈME + TOP PROCESSUS --------------------
        TileFrame {
            tileId: "system"; cellW: canvas.cellW; cellH: canvas.cellH
            Panel {
                anchors.fill: parent
                title: "SYSTÈME"
                tag: Metrics.network.ipAddress
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0
                    KV { Layout.fillWidth: true; k: "VOLUME"
                         v: Metrics.volume.muted ? "MUTE" : Metrics.volume.level.toFixed(0) + " %"
                         vc: Metrics.volume.muted ? Theme.crit : Theme.textHi }
                    KV { Layout.fillWidth: true; k: "PROCESSUS"; v: "" + Metrics.process.count }
                    KV { Layout.fillWidth: true; k: "THREADS"; v: "" + Metrics.process.threadCount }
                    Text {
                        Layout.fillWidth: true; Layout.topMargin: 8
                        text: "SON · " + (Metrics.volume.deviceName !== "" ? Metrics.volume.deviceName : "—")
                        color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 10
                        font.letterSpacing: 1; elide: Text.ElideRight
                    }
                    Item {
                        Layout.fillWidth: true; implicitHeight: 6
                        Rectangle { anchors.fill: parent; color: Qt.rgba(1, 1, 1, 0.06) }
                        Rectangle {
                            height: parent.height
                            width: parent.width * Math.max(0, Math.min(1, Metrics.volume.peakLevel))
                            gradient: Gradient {
                                orientation: Gradient.Horizontal
                                GradientStop { position: 0.0; color: Theme.accent }
                                GradientStop { position: 1.0; color: Theme.warn }
                            }
                            Behavior on width { NumberAnimation { duration: 60 } }
                        }
                    }

                    // --- Top processus (titre secondaire dans la même zone) ---
                    Text {
                        Layout.fillWidth: true; Layout.topMargin: 12
                        text: "TOP PROCESSUS"
                        color: Theme.muted; font.family: Theme.fontUi; font.pixelSize: 10
                        font.letterSpacing: 1.8; font.weight: Font.DemiBold
                    }
                    ListView {
                        id: procList
                        Layout.fillWidth: true; Layout.fillHeight: true
                        Layout.topMargin: 4
                        model: TaskKiller
                        interactive: false      // zone d'affichage, pas une liste à parcourir
                        clip: true

                        // ⚠️ refresh() énumère TOUS les processus (snapshot Toolhelp +
                        // un OpenProcess chacun, ~250 ici) : bien trop lourd pour le
                        // tick de 2 s des métriques → 5 s, et seulement si visible.
                        Component.onCompleted: TaskKiller.refresh()
                        Timer {
                            interval: 5000
                            repeat: true
                            running: procList.visible
                            onTriggered: TaskKiller.refresh()
                        }

                        delegate: Item {
                            required property string name
                            required property double memoryMb
                            required property bool critical

                            width: ListView.view.width
                            height: 22

                            Text {
                                anchors.left: parent.left
                                anchors.right: memT.left
                                anchors.rightMargin: 8
                                anchors.verticalCenter: parent.verticalCenter
                                text: parent.name
                                color: parent.critical ? Theme.muted : Theme.text
                                font.family: Theme.fontUi; font.pixelSize: 11
                                elide: Text.ElideRight
                            }
                            Text {
                                id: memT
                                anchors.right: parent.right
                                anchors.verticalCenter: parent.verticalCenter
                                text: parent.memoryMb >= 1024
                                      ? (parent.memoryMb / 1024).toFixed(1) + " Go"
                                      : Math.round(parent.memoryMb) + " Mo"
                                color: parent.memoryMb >= 1024 ? Theme.warn : Theme.textHi
                                font.family: Theme.fontMono; font.pixelSize: 11
                                font.weight: Font.DemiBold; font.features: ({ "tnum": 1 })
                            }
                        }
                    }
                }
            }
        }
    }
}
