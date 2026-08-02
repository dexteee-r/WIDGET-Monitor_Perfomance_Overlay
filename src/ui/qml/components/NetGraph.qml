import QtQuick
import QtQuick.Shapes
import PerformanceOverlay

// Graphe réseau en miroir : descendant au-dessus de l'axe, montant en dessous.
// Les deux séries partagent la MÊME échelle (max des deux) — sinon 90 Ko/s
// d'upload paraîtraient aussi hauts que 4 Mo/s de download, ce qui mentirait.
// Les aires sont dégradées vers l'axe : la lecture se fait sur le contour, le
// remplissage ne sert qu'à donner du corps.
Item {
    id: g

    property var down: []
    property var up: []
    property int capacity: 60
    property color downColor: Theme.accent
    property color upColor: Theme.accent2

    readonly property real _axis: g.height / 2

    readonly property real _max: {
        let m = 0
        for (let i = 0; i < g.down.length; i++) m = Math.max(m, g.down[i])
        for (let j = 0; j < g.up.length; j++) m = Math.max(m, g.up[j])
        return m > 0 ? m : 1
    }

    // Construit l'aire fermée d'une série. dir = -1 (vers le haut) ou +1 (bas).
    // Les échantillons sont calés à DROITE : l'instant présent est au bord droit.
    function area(values, dir) {
        const pts = []
        const n = values.length
        if (n === 0 || g.width <= 0)
            return pts
        const step = g.width / Math.max(1, g.capacity - 1)
        const x0 = g.width - (n - 1) * step
        const half = g._axis - 2

        pts.push(Qt.point(x0, g._axis))
        for (let i = 0; i < n; i++) {
            const h = half * Math.min(1, values[i] / g._max)
            pts.push(Qt.point(x0 + i * step, g._axis + dir * h))
        }
        pts.push(Qt.point(g.width, g._axis))
        return pts
    }

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        // Descendant (au-dessus de l'axe)
        ShapePath {
            strokeColor: g.downColor
            strokeWidth: 1.5
            capStyle: ShapePath.RoundCap
            fillGradient: LinearGradient {
                x1: 0; y1: 0; x2: 0; y2: g._axis
                GradientStop { position: 0.0; color: Qt.rgba(g.downColor.r, g.downColor.g, g.downColor.b, 0.35) }
                GradientStop { position: 1.0; color: Qt.rgba(g.downColor.r, g.downColor.g, g.downColor.b, 0.02) }
            }
            PathPolyline { path: g.area(g.down, -1) }
        }

        // Montant (en dessous de l'axe)
        ShapePath {
            strokeColor: g.upColor
            strokeWidth: 1.5
            capStyle: ShapePath.RoundCap
            fillGradient: LinearGradient {
                x1: 0; y1: g._axis; x2: 0; y2: g.height
                GradientStop { position: 0.0; color: Qt.rgba(g.upColor.r, g.upColor.g, g.upColor.b, 0.30) }
                GradientStop { position: 1.0; color: Qt.rgba(g.upColor.r, g.upColor.g, g.upColor.b, 0.02) }
            }
            PathPolyline { path: g.area(g.up, 1) }
        }
    }

    // Axe médian
    Rectangle {
        anchors.left: parent.left; anchors.right: parent.right
        y: g._axis
        height: 1
        color: Theme.border2
    }
}
