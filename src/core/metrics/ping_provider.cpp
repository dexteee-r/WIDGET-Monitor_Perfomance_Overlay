#include "ping_provider.h"

#include <QThread>
#include <QStringList>
#include <vector>

// winsock2.h DOIT précéder iphlpapi/icmpapi (types d'adresses).
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>

namespace {

constexpr int kTimeoutMs = 1000;
constexpr int kLossWindow = 30;   // fenêtre du taux de perte (≈ 1 min à 2 s/tick)

// Parse "a.b.c.d" → IPAddr (ordre réseau) sans dépendre de ws2_32 (inet_addr).
bool parseIpv4(const QString &text, IPAddr *out)
{
    const QStringList parts = text.split(QLatin1Char('.'));
    if (parts.size() != 4)
        return false;
    quint32 addr = 0;
    for (int i = 0; i < 4; ++i) {
        bool ok = false;
        const uint v = parts.at(i).toUInt(&ok);
        if (!ok || v > 255)
            return false;
        addr |= (v << (8 * i));   // little-endian host → octets en ordre réseau
    }
    *out = static_cast<IPAddr>(addr);
    return true;
}

} // namespace

// ---------------------------------------------------------------------------
// Worker : vit sur son propre thread, fait l'appel ICMP bloquant.
// ---------------------------------------------------------------------------
class PingWorker : public QObject
{
    Q_OBJECT

public slots:
    void doPing(const QString &host)
    {
        IPAddr addr = 0;
        if (!parseIpv4(host, &addr)) {
            emit finished(0, false);
            return;
        }

        const HANDLE h = IcmpCreateFile();
        if (h == INVALID_HANDLE_VALUE) {
            emit finished(0, false);
            return;
        }

        char payload[32] = "PerformanceOverlay";
        // Le buffer doit tenir la réponse + la charge utile (+8 = marge exigée
        // par l'API pour les données de contrôle ICMP).
        std::vector<char> reply(sizeof(ICMP_ECHO_REPLY) + sizeof(payload) + 8);

        const DWORD n = IcmpSendEcho(h, addr, payload, sizeof(payload), nullptr,
                                     reply.data(), static_cast<DWORD>(reply.size()),
                                     kTimeoutMs);
        int latency = 0;
        bool ok = false;
        if (n > 0) {
            const auto *r = reinterpret_cast<ICMP_ECHO_REPLY *>(reply.data());
            ok = (r->Status == IP_SUCCESS);
            if (ok)
                latency = static_cast<int>(r->RoundTripTime);
        }
        IcmpCloseHandle(h);

        emit finished(latency, ok);
    }

signals:
    void finished(int latencyMs, bool ok);
};

// ---------------------------------------------------------------------------

PingProvider::PingProvider(QObject *parent)
    : MetricProvider(parent)
{
    m_thread = new QThread(this);
    auto *worker = new PingWorker;
    worker->moveToThread(m_thread);

    connect(m_thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &PingProvider::requestPing, worker, &PingWorker::doPing);
    connect(worker, &PingWorker::finished, this, &PingProvider::onResult);

    m_thread->start();
}

PingProvider::~PingProvider()
{
    if (m_thread) {
        m_thread->quit();
        // Le worker peut être dans un IcmpSendEcho bloquant : on lui laisse le
        // temps du timeout + marge avant de rendre la main.
        m_thread->wait(kTimeoutMs + 500);
    }
}

void PingProvider::setHost(const QString &host)
{
    if (host.isEmpty() || host == m_host)
        return;
    m_host = host;
    m_sent = 0;
    m_lost = 0;
    emit hostChanged();
}

void PingProvider::poll()
{
    // Un seul ping en vol : si le précédent n'a pas répondu (réseau coupé), on
    // n'empile pas les demandes.
    if (m_inFlight)
        return;
    m_inFlight = true;
    emit requestPing(m_host);
}

void PingProvider::onResult(int latencyMs, bool ok)
{
    m_inFlight = false;
    m_reachable = ok;
    m_latencyMs = ok ? latencyMs : 0;

    // Une perte pousse 0 dans l'historique : le trou est visible sur le graphe.
    m_hist.push(ok ? latencyMs : 0);

    if (m_sent < kLossWindow) {
        ++m_sent;
        if (!ok)
            ++m_lost;
    } else {
        // Fenêtre glissante approchée : on fait décroître doucement le compteur
        // de pertes pour que le taux reflète l'état récent, pas tout l'uptime.
        if (!ok)
            m_lost = qMin(kLossWindow, m_lost + 1);
        else
            m_lost = qMax(0, m_lost - 1);
    }
    m_lossPercent = m_sent > 0 ? (100.0 * m_lost / m_sent) : 0.0;

    emit sampleChanged();
}

#include "ping_provider.moc"
