#pragma once

#include <QtQml/qqmlregistration.h>
#include <QString>
#include <QVariantList>
#include "metric_provider.h"
#include "history.h"

class QThread;

// Provider Ping : latence ICMP vers un hôte (défaut 1.1.1.1) + taux de perte
// sur la fenêtre récente.
//
// ⚠️ IcmpSendEcho est BLOQUANT (jusqu'au timeout). L'appeler sur le thread UI
// figerait l'overlay à chaque paquet perdu → le ping tourne sur un thread
// worker dédié, `poll()` ne fait qu'y poster une demande (connexion en queue)
// et le résultat revient par signal. Un seul ping en vol à la fois.
class PingProvider : public MetricProvider
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Fourni par le singleton Metrics")

    Q_PROPERTY(QString host READ host NOTIFY hostChanged)
    Q_PROPERTY(int latencyMs READ latencyMs NOTIFY sampleChanged)
    Q_PROPERTY(bool reachable READ reachable NOTIFY sampleChanged)
    Q_PROPERTY(double lossPercent READ lossPercent NOTIFY sampleChanged)
    Q_PROPERTY(QVariantList latencyHistory READ latencyHistory NOTIFY sampleChanged)

public:
    explicit PingProvider(QObject *parent = nullptr);
    ~PingProvider() override;

    QString host() const { return m_host; }
    int latencyMs() const { return m_latencyMs; }
    bool reachable() const { return m_reachable; }
    double lossPercent() const { return m_lossPercent; }
    QVariantList latencyHistory() const { return m_hist.toVariantList(); }

    Q_INVOKABLE void setHost(const QString &host);

    void poll() override;

signals:
    void hostChanged();
    void sampleChanged();
    void requestPing(const QString &host);   // → worker (connexion en queue)

private:
    void onResult(int latencyMs, bool ok);

    QString m_host = QStringLiteral("1.1.1.1");
    int m_latencyMs = 0;
    bool m_reachable = false;
    double m_lossPercent = 0.0;
    History m_hist;

    // Fenêtre glissante de succès/échecs pour le taux de perte.
    int m_sent = 0;
    int m_lost = 0;

    bool m_inFlight = false;
    QThread *m_thread = nullptr;
};
