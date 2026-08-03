#pragma once

#include <QtQml/qqmlregistration.h>
#include <QString>
#include <QElapsedTimer>
#include "metric_provider.h"

class QNetworkAccessManager;
class QNetworkReply;

// Provider Météo : conditions courantes + min/max du jour via Open-Meteo.
// Choisi parce que l'API est publique, sans clé et sans compte (même esprit que
// le scraping mawaqit : zéro dépendance, zéro secret à embarquer dans le dépôt).
// Requête GET uniquement, aucune donnée personnelle envoyée — juste des
// coordonnées. Rafraîchi ≤ 1×/15 min : la météo ne bouge pas à 0,5 Hz.
class WeatherProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(bool valid READ valid NOTIFY dataChanged)
    Q_PROPERTY(QString location READ location NOTIFY dataChanged)
    Q_PROPERTY(double temperatureC READ temperatureC NOTIFY dataChanged)
    Q_PROPERTY(double apparentC READ apparentC NOTIFY dataChanged)
    Q_PROPERTY(double minC READ minC NOTIFY dataChanged)
    Q_PROPERTY(double maxC READ maxC NOTIFY dataChanged)
    Q_PROPERTY(double windKph READ windKph NOTIFY dataChanged)
    Q_PROPERTY(QString description READ description NOTIFY dataChanged)
    Q_PROPERTY(QString symbol READ symbol NOTIFY dataChanged)
    // Code WMO brut : c'est lui qui pilote l'illustration animée côté QML.
    // Le pictogramme texte ne suffisait pas — impossible d'en déduire la famille
    // de temps de façon fiable.
    Q_PROPERTY(int code READ code NOTIFY dataChanged)

public:
    explicit WeatherProvider(QObject *parent = nullptr);

    bool valid() const { return m_valid; }
    QString location() const { return m_location; }
    double temperatureC() const { return m_temp; }
    double apparentC() const { return m_apparent; }
    double minC() const { return m_min; }
    double maxC() const { return m_max; }
    double windKph() const { return m_wind; }
    QString description() const { return m_desc; }
    QString symbol() const { return m_symbol; }
    int code() const { return m_code; }

    // Permet de suivre une autre ville sans recompiler (appelable depuis QML).
    Q_INVOKABLE void configure(double latitude, double longitude, const QString &label);

    void poll() override;

signals:
    void dataChanged();

private:
    void fetch();
    void onReply(QNetworkReply *reply);
    static QString describe(int wmoCode, QString *symbolOut);

    QNetworkAccessManager *m_net = nullptr;

    // Défaut : Jodoigne (Brabant wallon) — même ville que les horaires de prière.
    double m_lat = 50.7239;
    double m_lon = 4.8697;
    QString m_location = QStringLiteral("Jodoigne");

    bool m_valid = false;
    double m_temp = 0.0;
    double m_apparent = 0.0;
    double m_min = 0.0;
    double m_max = 0.0;
    double m_wind = 0.0;
    QString m_desc;
    QString m_symbol;
    int m_code = -1;

    QElapsedTimer m_since;
    bool m_inFlight = false;
};
