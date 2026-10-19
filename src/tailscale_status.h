#pragma once

#include <QDateTime>
#include <QMap>
#include <QString>
#include <QVariantMap>
#include <QVector>

namespace tailscale {

struct Peer {
    QString publicKey;
    QString id;
    QString dnsName;
    QString hostName;
    bool online = false;
    bool active = false;
    QDateTime lastSeen;
};

struct Status {
    QString backendState;
    QVector<QString> health;
    QMap<QString, Peer> peers;
};

enum class PeerDisplayState { Connected, Disconnected };

Status parseStatus(const QByteArray &json);

PeerDisplayState mapPeerState(const Peer &peer);

const Peer *findPeer(const Status &status, const QString &device);

QVariantMap peerEntry(const Peer &peer);

QVariantMap statusToMap(const Status &status);

}
