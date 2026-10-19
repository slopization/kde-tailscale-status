import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

KCM.SimpleKCM {
    property alias cfg_devices: devicesField.text
    property alias cfg_pollingInterval: intervalField.value

    Kirigami.FormLayout {
        QQC2.TextField {
            id: devicesField
            Kirigami.FormData.label: "Devices:"
            placeholderText: "node id or dns name, comma separated"
        }
        QQC2.SpinBox {
            id: intervalField
            from: 1
            to: 300
            Kirigami.FormData.label: "Polling interval (s):"
        }
    }
}
