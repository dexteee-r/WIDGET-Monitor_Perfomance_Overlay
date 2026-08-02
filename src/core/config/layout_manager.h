#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

// Disposition des zones du cockpit, éditable par l'utilisateur et persistée.
//
// Les positions sont exprimées en CELLULES d'une grille (24 × 16), pas en
// pixels : la même disposition vaut donc pour le cockpit (1280×720) et pour le
// plein écran (2560×1440), qui partagent le même rapport 16/9. Une zone occupe
// {x, y, w, h} cellules.
//
// Persisté dans config.ini, section [Layout], une clé par zone :
//   cpu=0,0,7,5
// Singleton QML `Layouts`.
class LayoutManager : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Layouts)
    QML_SINGLETON

    Q_PROPERTY(int columns READ columns CONSTANT)
    Q_PROPERTY(int rows READ rows CONSTANT)
    // Une seule notification pour toute la table : les déplacements sont rares
    // (action utilisateur), inutile d'émettre un signal par zone.
    Q_PROPERTY(QVariantMap tiles READ tiles NOTIFY tilesChanged)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)

public:
    explicit LayoutManager(QObject *parent = nullptr);

    int columns() const { return kColumns; }
    int rows() const { return kRows; }
    QVariantMap tiles() const { return m_tiles; }

    bool editMode() const { return m_editMode; }
    void setEditMode(bool on);

    // Géométrie d'une zone : [x, y, w, h] en cellules. Liste vide si inconnue.
    Q_INVOKABLE QVariantList tile(const QString &id) const;

    // Déplace/redimensionne une zone. Les valeurs sont bornées à la grille et
    // la zone garde au moins 2×2 cellules (en dessous, plus rien n'est lisible).
    Q_INVOKABLE void setTile(const QString &id, int x, int y, int w, int h);

    // Restaure la disposition d'origine (et l'écrit dans config.ini).
    Q_INVOKABLE void reset();

signals:
    void tilesChanged();
    void editModeChanged();

private:
    static constexpr int kColumns = 24;
    static constexpr int kRows = 16;
    static constexpr int kMinSpan = 2;

    static QVariantMap defaults();
    void load();
    void save(const QString &id);

    QString m_path;
    QVariantMap m_tiles;
    bool m_editMode = false;
};
