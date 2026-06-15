import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: "#1e1e2e"
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 10; spacing: 8
        Text { text: "Game State"; color: "#a6e3a1"; font.pixelSize: 14; font.bold: true }
        Label {
            text: "Live state display will appear here when connected"
            color: "#6c7086"; font.pixelSize: 12; wrapMode: Text.WordWrap; Layout.fillWidth: true
            visible: !mainWindow.isConnected
        }
        Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; color: "#181825"; radius: 4; visible: mainWindow.isConnected }
    }
}
