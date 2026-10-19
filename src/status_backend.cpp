#include "status_backend.h"

#include "tailscale_status.h"

#include <stdexcept>

StatusBackend::StatusBackend(QObject *parent) : QObject(parent) {
    connect(&m_process, &QProcess::finished, this, [this](int exitCode) {
        if (exitCode != 0) {
            emit failed(QString::fromUtf8(m_process.readAllStandardError()).trimmed());
            return;
        }
        try {
            m_status = tailscale::parseStatus(m_process.readAllStandardOutput());
            m_ready = true;
            emit statusReady(tailscale::statusToMap(m_status));
        } catch (const std::invalid_argument &error) {
            emit failed(QString::fromUtf8(error.what()));
        }
    });
    connect(&m_timer, &QTimer::timeout, this, [this] {
        poll();
    });
}

void StatusBackend::start() {
    poll();
    m_timer.start(m_pollingInterval * 1000);
}

void StatusBackend::poll() {
    if (m_process.state() != QProcess::NotRunning)
        return;
    m_process.start("tailscale", {"status", "--json"});
}

int StatusBackend::pollingInterval() const {
    return m_pollingInterval;
}

void StatusBackend::setPollingInterval(int seconds) {
    if (seconds == m_pollingInterval)
        return;
    m_pollingInterval = seconds;
    m_timer.setInterval(seconds * 1000);
    emit pollingIntervalChanged();
}

QVariantMap StatusBackend::peerForDevice(const QString &device) const {
    if (!m_ready)
        return QVariantMap();
    const tailscale::Peer *peer = tailscale::findPeer(m_status, device);
    if (!peer)
        return QVariantMap();
    return tailscale::peerEntry(*peer);
}
