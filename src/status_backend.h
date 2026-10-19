#pragma once

#include <QtQmlIntegration>

#include "tailscale_status.h"

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QVariantMap>

class StatusBackend : public QObject {
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int pollingInterval READ pollingInterval WRITE setPollingInterval NOTIFY pollingIntervalChanged)

public:
    explicit StatusBackend(QObject *parent = nullptr);

    int pollingInterval() const;
    void setPollingInterval(int seconds);

    Q_INVOKABLE void start();
    Q_INVOKABLE QVariantMap peerForDevice(const QString &device) const;

signals:
    void statusReady(const QVariantMap &status);
    void failed(const QString &message);
    void pollingIntervalChanged();

private:
    void poll();

    QProcess m_process;
    QTimer m_timer;
    int m_pollingInterval = 5;
    tailscale::Status m_status;
    bool m_ready = false;
};
