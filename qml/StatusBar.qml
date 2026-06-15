import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: "#181825"; border.color: "#313244"
    RowLayout {
        anchors.fill: parent; anchors.leftMargin: 8; anchors.rightMargin: 8; spacing: 12
        Rectangle { width: 10; height: 10; radius: 5; color: mainWindow.isConnected ? "#a6e3a1" : "#f38ba8" }
        Text { text: mainWindow.statusText; color: "#a6adc8"; font.pixelSize: 11 }
        Rectangle { Layout.fillWidth: true }
        Text { text: mainWindow.translationStats; color: "#a6adc8"; font.pixelSize: 11 }
    }
}
