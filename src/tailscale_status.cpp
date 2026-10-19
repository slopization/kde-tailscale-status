#include "tailscale_status.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>

namespace tailscale {

Status parseStatus(const QByteArray &json) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (error.error != QJsonParseError::NoError)
        throw std::invalid_argument(error.errorString().toStdString());

    const QJsonObject root = doc.object();
    Status status;
    status.backendState = root.value("BackendState").toString();

    for (const QJsonValue &value : root.value("Health").toArray())
        status.health.append(value.toString());

    const QJsonObject peers = root.value("Peer").toObject();
    for (auto it = peers.begin(); it != peers.end(); ++it) {
        const QJsonObject obj = it.value().toObject();
        Peer peer;
        peer.publicKey = it.key();
        peer.id = obj.value("ID").toString();
        peer.dnsName = obj.value("DNSName").toString();
        peer.hostName = obj.value("HostName").toString();
        peer.online = obj.value("Online").toBool();
        peer.active = obj.value("Active").toBool();
        peer.lastSeen = QDateTime::fromString(obj.value("LastSeen").toString(), Qt::ISODate);
        status.peers.insert(it.key(), peer);
    }

    return status;
}

PeerDisplayState mapPeerState(const Peer &peer) {
    return peer.online ? PeerDisplayState::Connected : PeerDisplayState::Disconnected;
}

const Peer *findPeer(const Status &status, const QString &device) {
    const QString target = device.toLower();
    for (auto it = status.peers.constBegin(); it != status.peers.constEnd(); ++it) {
        const Peer &peer = it.value();
        const QString dns = peer.dnsName.toLower();
        if (peer.id.toLower() == target || dns == target || dns == target + '.')
            return &peer;
    }
    return nullptr;
}

QVariantMap peerEntry(const Peer &peer) {
    QVariantMap entry;
    entry.insert("id", peer.id);
    entry.insert("dnsName", peer.dnsName);
    entry.insert("hostName", peer.hostName);
    entry.insert("online", peer.online);
    entry.insert("active", peer.active);
    entry.insert("lastSeen", peer.lastSeen.toString(Qt::ISODate));
    entry.insert("state", mapPeerState(peer) == PeerDisplayState::Connected ? "connected" : "disconnected");
    return entry;
}

QVariantMap statusToMap(const Status &status) {
    QVariantMap map;
    map.insert("backendState", status.backendState);

    QVariantList health;
    for (const QString &problem : status.health)
        health.append(problem);
    map.insert("health", health);

    QVariantMap peers;
    for (auto it = status.peers.constBegin(); it != status.peers.constEnd(); ++it)
        peers.insert(it.key(), peerEntry(it.value()));
    map.insert("peers", peers);

    return map;
}

}
