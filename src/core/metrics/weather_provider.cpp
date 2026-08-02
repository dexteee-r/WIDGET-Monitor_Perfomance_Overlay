#include "weather_provider.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QUrlQuery>

namespace {
constexpr qint64 kRefreshMs = 15 * 60 * 1000;   // 15 min
}

WeatherProvider::WeatherProvider(QObject *parent)
    : MetricProvider(parent)
    , m_net(new QNetworkAccessManager(this))
{
    connect(m_net, &QNetworkAccessManager::finished, this, &WeatherProvider::onReply);
    m_desc = QStringLiteral("—");
    m_symbol = QStringLiteral("·");
}

void WeatherProvider::configure(double latitude, double longitude, const QString &label)
{
    m_lat = latitude;
    m_lon = longitude;
    if (!label.isEmpty())
        m_location = label;
    m_since.invalidate();   // force un rafraîchissement au prochain tick
    emit dataChanged();
}

void WeatherProvider::poll()
{
    // Piloté par le tick global (2 s) mais throttlé : on ne sort sur le réseau
    // que si les données ont plus de 15 min, ou au tout premier passage.
    if (m_inFlight)
        return;
    if (m_since.isValid() && m_since.elapsed() < kRefreshMs)
        return;
    fetch();
}

void WeatherProvider::fetch()
{
    QUrl url(QStringLiteral("https://api.open-meteo.com/v1/forecast"));
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("latitude"), QString::number(m_lat, 'f', 4));
    q.addQueryItem(QStringLiteral("longitude"), QString::number(m_lon, 'f', 4));
    q.addQueryItem(QStringLiteral("current"),
                   QStringLiteral("temperature_2m,apparent_temperature,weather_code,wind_speed_10m"));
    q.addQueryItem(QStringLiteral("daily"),
                   QStringLiteral("temperature_2m_max,temperature_2m_min"));
    q.addQueryItem(QStringLiteral("timezone"), QStringLiteral("Europe/Brussels"));
    q.addQueryItem(QStringLiteral("forecast_days"), QStringLiteral("1"));
    url.setQuery(q);

    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("PerformanceOverlay"));
    m_inFlight = true;
    m_net->get(req);
}

void WeatherProvider::onReply(QNetworkReply *reply)
{
    reply->deleteLater();
    m_inFlight = false;

    if (reply->error() != QNetworkReply::NoError) {
        // Pas de réseau : on garde la dernière valeur connue et on retentera au
        // prochain cycle (le timer ne redémarre pas → nouvelle tentative au tick).
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject())
        return;

    const QJsonObject root = doc.object();
    const QJsonObject cur = root.value(QStringLiteral("current")).toObject();
    if (cur.isEmpty())
        return;

    m_temp = cur.value(QStringLiteral("temperature_2m")).toDouble();
    m_apparent = cur.value(QStringLiteral("apparent_temperature")).toDouble();
    m_wind = cur.value(QStringLiteral("wind_speed_10m")).toDouble();
    m_desc = describe(cur.value(QStringLiteral("weather_code")).toInt(), &m_symbol);

    const QJsonObject daily = root.value(QStringLiteral("daily")).toObject();
    const QJsonArray maxArr = daily.value(QStringLiteral("temperature_2m_max")).toArray();
    const QJsonArray minArr = daily.value(QStringLiteral("temperature_2m_min")).toArray();
    if (!maxArr.isEmpty())
        m_max = maxArr.at(0).toDouble();
    if (!minArr.isEmpty())
        m_min = minArr.at(0).toDouble();

    m_valid = true;
    m_since.start();
    emit dataChanged();
}

// Codes WMO (documentés par Open-Meteo) → libellé court FR + pictogramme texte.
QString WeatherProvider::describe(int code, QString *symbolOut)
{
    struct Entry { int lo; int hi; const char *label; const char *symbol; };
    static const Entry table[] = {
        {  0,  0, "Ciel dégagé",      "○" },
        {  1,  1, "Peu nuageux",      "◔" },
        {  2,  2, "Partiellement couvert", "◑" },
        {  3,  3, "Couvert",          "●" },
        { 45, 48, "Brouillard",       "≡" },
        { 51, 57, "Bruine",           "⁘" },
        { 61, 67, "Pluie",            "☂" },
        { 71, 77, "Neige",            "❄" },
        { 80, 82, "Averses",          "☂" },
        { 85, 86, "Averses de neige", "❄" },
        { 95, 99, "Orage",            "⚡" },
    };
    for (const Entry &e : table) {
        if (code >= e.lo && code <= e.hi) {
            if (symbolOut)
                *symbolOut = QString::fromUtf8(e.symbol);
            return QString::fromUtf8(e.label);
        }
    }
    if (symbolOut)
        *symbolOut = QStringLiteral("·");
    return QStringLiteral("—");
}
