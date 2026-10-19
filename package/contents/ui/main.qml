import QtQuick
import QtQuick.Layouts
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaControls3
import org.kde.kirigami as Kirigami
import org.kde.tailscale_status

PlasmoidItem {
    id: root

    property var status: ({})
    property var errorMessage: ""
    property var devices: (plasmoid.configuration.devices ?? "").split(",").map(s => s.trim()).filter(s => s.length > 0)

    implicitWidth: Kirigami.Units.gridUnit * 14
    implicitHeight: layout.implicitHeight + Kirigami.Units.smallSpacing * 2
    height: implicitHeight

    StatusBackend {
        id: backend
        pollingInterval: plasmoid.configuration.pollingInterval || 5
        onStatusReady: (status) => {
            root.status = status
            root.errorMessage = ""
        }
        onFailed: (message) => {
            root.errorMessage = message
        }
        Component.onCompleted: start()
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: Kirigami.Units.smallSpacing
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            model: root.devices
            delegate: RowLayout {
                property var peer: {
                    root.status
                    const p = backend.peerForDevice(modelData)
                    return p.id === undefined ? null : p
                }

                Rectangle {
                    implicitWidth: 10
                    implicitHeight: 10
                    radius: 5
                    color: peer ? (peer.state === "connected" ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor) : Kirigami.Theme.neutralTextColor
                }
                PlasmaControls3.Label {
                    Layout.fillWidth: true
                    text: peer ? (peer.dnsName.split(".")[0] || peer.hostName) : modelData
                }
                PlasmaControls3.Label {
                    color: peer && peer.state === "connected" ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
                    text: peer && peer.state === "connected" ? "active" : "inactive"
                }
            }
        }

        PlasmaControls3.Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            color: Kirigami.Theme.negativeTextColor
            text: root.errorMessage !== "" ? root.errorMessage : (root.status.health ?? []).join(", ")
            visible: text !== ""
        }
    }
}
