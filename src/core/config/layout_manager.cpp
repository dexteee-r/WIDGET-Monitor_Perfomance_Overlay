#include "layout_manager.h"

#include <QCoreApplication>
#include <QSettings>
#include <QStringList>

namespace {

QVariantList cell(int x, int y, int w, int h)
{
    return QVariantList{x, y, w, h};
}

} // namespace

LayoutManager::LayoutManager(QObject *parent)
    : QObject(parent)
{
    m_path = QCoreApplication::applicationDirPath() + QStringLiteral("/config.ini");
    load();
}

// Disposition par défaut = celle composée par l'utilisateur en mode édition
// (2026-08-02), relevée dans config.ini et gravée ici pour qu'elle survive à un
// RAZ, à un config.ini neuf et à un redéploiement.
// Colonnes : matériel 0-6, centre 7-16, contexte 17-23.
QVariantMap LayoutManager::defaults()
{
    QVariantMap m;
    // Colonne matériel
    m.insert(QStringLiteral("cpu"),     cell(0, 0, 7, 6));
    m.insert(QStringLiteral("gpu"),     cell(0, 6, 7, 6));
    m.insert(QStringLiteral("memory"),  cell(0, 12, 7, 4));
    // Colonne centrale
    m.insert(QStringLiteral("flux"),    cell(7, 0, 10, 9));
    // Stockage tient en 3 rangées depuis que chaque disque occupe une seule ligne.
    m.insert(QStringLiteral("storage"), cell(7, 9, 10, 3));
    m.insert(QStringLiteral("network"), cell(7, 12, 10, 4));
    // Colonne contexte
    m.insert(QStringLiteral("prayer"),  cell(17, 0, 3, 4));
    m.insert(QStringLiteral("weather"), cell(20, 0, 4, 4));
    m.insert(QStringLiteral("system"),  cell(17, 4, 7, 7));
    // Chrono : au moins 4 rangées, sinon la tuile rogne ses boutons GO/RAZ.
    m.insert(QStringLiteral("timer"),   cell(17, 11, 3, 5));
    return m;
}

void LayoutManager::load()
{
    m_tiles = defaults();

    QSettings s(m_path, QSettings::IniFormat);
    s.beginGroup(QStringLiteral("Layout"));
    const QStringList keys = s.childKeys();
    for (const QString &id : keys) {
        // Format "x,y,w,h" — on ignore silencieusement toute entrée mal formée
        // plutôt que de casser le démarrage sur un config.ini édité à la main.
        const QStringList parts = s.value(id).toString().split(QLatin1Char(','));
        if (parts.size() != 4)
            continue;
        bool ok = true;
        int v[4];
        for (int i = 0; i < 4 && ok; ++i)
            v[i] = parts.at(i).trimmed().toInt(&ok);
        if (!ok)
            continue;
        m_tiles.insert(id, cell(v[0], v[1], v[2], v[3]));
    }
    s.endGroup();
}

void LayoutManager::save(const QString &id)
{
    const QVariantList g = m_tiles.value(id).toList();
    if (g.size() != 4)
        return;
    QSettings s(m_path, QSettings::IniFormat);
    s.beginGroup(QStringLiteral("Layout"));
    s.setValue(id, QStringLiteral("%1,%2,%3,%4")
                       .arg(g.at(0).toInt()).arg(g.at(1).toInt())
                       .arg(g.at(2).toInt()).arg(g.at(3).toInt()));
    s.endGroup();
}

QVariantList LayoutManager::tile(const QString &id) const
{
    return m_tiles.value(id).toList();
}

void LayoutManager::setTile(const QString &id, int x, int y, int w, int h)
{
    if (!m_tiles.contains(id))
        return;

    // Bornage : une zone reste entière dans la grille et jamais plus petite
    // que kMinSpan (en dessous, le contenu n'est plus lisible du tout).
    w = qBound(kMinSpan, w, kColumns);
    h = qBound(kMinSpan, h, kRows);
    x = qBound(0, x, kColumns - w);
    y = qBound(0, y, kRows - h);

    const QVariantList next = cell(x, y, w, h);
    if (m_tiles.value(id).toList() == next)
        return;

    m_tiles.insert(id, next);
    save(id);
    emit tilesChanged();
}

void LayoutManager::reset()
{
    m_tiles = defaults();

    QSettings s(m_path, QSettings::IniFormat);
    s.beginGroup(QStringLiteral("Layout"));
    s.remove(QString());   // efface toute la section : on repart des défauts
    s.endGroup();

    emit tilesChanged();
}

void LayoutManager::setEditMode(bool on)
{
    if (on == m_editMode)
        return;
    m_editMode = on;
    emit editModeChanged();
}
