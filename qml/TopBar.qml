import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: "#181825"; border.color: "#313244"
    RowLayout {
        anchors.fill: parent; anchors.margins: 6; spacing: 8
        Text { text: "RTranslator"; color: "#cba6f7"; font.pixelSize: 16; font.bold: true }
        Rectangle { width: 1; height: 28; color: "#313244" }
        TextField {
            id: pathField; Layout.fillWidth: true; placeholderText: "Game path..."
            text: mainWindow.gamePath; color: "#cdd6f4"
            background: Rectangle { color: "#313244"; radius: 4 }
            onTextChanged: mainWindow.setGamePath(text)
        }
        Button {
            text: "Launch"; enabled: !mainWindow.isConnected && mainWindow.gamePath !== ""
            onClicked: mainWindow.launchGame()
            background: Rectangle { color: parent.enabled ? "#a6e3a1" : "#45475a"; radius: 4 }
        }
        Button {
            text: "Detach"; enabled: mainWindow.isConnected
            onClicked: mainWindow.detachGame()
            background: Rectangle { color: parent.enabled ? "#f38ba8" : "#45475a"; radius: 4 }
        }
    }
}
