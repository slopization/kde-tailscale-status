#include <QtTest>

#include "../src/tailscale_status.h"

using namespace tailscale;

class StatusTest : public QObject {
    Q_OBJECT

private slots:
    void parsesPeersByKeyAndIdentity();
    void onlinePeerMapsToConnected();
    void offlinePeerMapsToDisconnected();
    void parsesHealthAsStrings();
    void parsesLastSeenWhenOffline();
    void statusToMapCarriesMappedState();
    void findsPeerByDeviceWithoutTrailingDot();
    void returnsNullForUnknownDevice();
    void rejectsInvalidJson();
};

void StatusTest::parsesPeersByKeyAndIdentity() {
    const QByteArray json =
        R"({"BackendState":"Running","Peer":{"key1":{"ID":"id1","DNSName":"nas.example.com.","HostName":"nas-host"},"key2":{"ID":"id2","DNSName":"laptop.example.com."}}})";
    Status status = parseStatus(json);
    QCOMPARE(status.backendState, "Running");
    QCOMPARE(status.peers.count(), 2);
    QCOMPARE(status.peers["key1"].id, "id1");
    QCOMPARE(status.peers["key1"].hostName, "nas-host");
    QCOMPARE(status.peers["key2"].dnsName, "laptop.example.com.");
    QCOMPARE(status.peers["key1"].publicKey, "key1");
}

void StatusTest::onlinePeerMapsToConnected() {
    Peer peer;
    peer.online = true;
    QCOMPARE(mapPeerState(peer), PeerDisplayState::Connected);
}

void StatusTest::offlinePeerMapsToDisconnected() {
    Peer peer;
    peer.online = false;
    QCOMPARE(mapPeerState(peer), PeerDisplayState::Disconnected);
}

void StatusTest::parsesHealthAsStrings() {
    Status status = parseStatus(R"({"BackendState":"Running","Health":["https://tailscale.com/kb/1337/derp-timeout"]})");
    QCOMPARE(status.health.count(), 1);
    QCOMPARE(status.health[0], "https://tailscale.com/kb/1337/derp-timeout");
}

void StatusTest::parsesLastSeenWhenOffline() {
    Status status = parseStatus(R"({"Peer":{"key1":{"Online":false,"LastSeen":"2026-10-19T12:00:00Z"}}})");
    QCOMPARE(status.peers["key1"].lastSeen, QDateTime(QDate(2026, 10, 19), QTime(12, 0, 0), QTimeZone(QTimeZone::UTC)));
}

void StatusTest::statusToMapCarriesMappedState() {
    Status status = parseStatus(R"({"BackendState":"Running","Health":["https://tailscale.com/kb/1337/derp-timeout"],"Peer":{"key1":{"ID":"id1","Online":true}}})");
    QVariantMap map = statusToMap(status);
    QCOMPARE(map.value("backendState").toString(), "Running");
    QCOMPARE(map.value("health").value<QVariantList>().count(), 1);
    QCOMPARE(map.value("peers").toMap().value("key1").toMap().value("state").toString(), "connected");
}

void StatusTest::findsPeerByDeviceWithoutTrailingDot() {
    Status status = parseStatus(R"({"Peer":{"key1":{"ID":"id1","DNSName":"nas.example.com."}}})");
    const Peer *peer = findPeer(status, "nas.example.com");
    QVERIFY(peer);
    QCOMPARE(peer->id, "id1");
}

void StatusTest::returnsNullForUnknownDevice() {
    Status status = parseStatus(R"({"Peer":{"key1":{"ID":"id1","DNSName":"nas.example.com."}}})");
    QVERIFY(findPeer(status, "typo") == nullptr);
}

void StatusTest::rejectsInvalidJson() {
    QVERIFY_THROWS_EXCEPTION(std::invalid_argument, parseStatus("not json"));
}

int main(int argc, char **argv) {
    StatusTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "tailscale_status_test.moc"
