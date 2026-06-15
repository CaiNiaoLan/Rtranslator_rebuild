import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: "#1e1e2e"
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 10; spacing: 10
        Text { text: "Modification"; color: "#cba6f7"; font.pixelSize: 14; font.bold: true }
        RowLayout {
            Text { text: "Gold:"; color: "#cdd6f4"; Layout.preferredWidth: 60 }
            TextField { id: goldField; Layout.fillWidth: true; text: "0"; color: "#cdd6f4"; background: Rectangle { color: "#313244"; radius: 4 } }
            Button { text: "Set"; onClicked: mainWindow.writeGameState("party.gold", parseInt(goldField.text)) }
        }
        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }
        RowLayout {
            Text { text: "Switch:"; color: "#cdd6f4"; Layout.preferredWidth: 60 }
            TextField { id: switchField; Layout.fillWidth: true; text: "1"; color: "#cdd6f4"; background: Rectangle { color: "#313244"; radius: 4 } }
            Button { text: "ON"; onClicked: mainWindow.writeGameState("switches." + switchField.text, true) }
            Button { text: "OFF"; onClicked: mainWindow.writeGameState("switches." + switchField.text, false) }
        }
        RowLayout {
            Text { text: "Var:"; color: "#cdd6f4"; Layout.preferredWidth: 60 }
            TextField { id: varIdField; Layout.fillWidth: true; text: "1"; color: "#cdd6f4"; background: Rectangle { color: "#313244"; radius: 4 } }
            TextField { id: varValField; Layout.fillWidth: true; text: "0"; color: "#cdd6f4"; background: Rectangle { color: "#313244"; radius: 4 } }
            Button { text: "Set"; onClicked: mainWindow.writeGameState("variables." + varIdField.text, parseInt(varValField.text)) }
        }
        RowLayout {
            Text { text: "HP:"; color: "#cdd6f4"; Layout.preferredWidth: 60 }
            TextField { id: hpField; Layout.fillWidth: true; text: "0"; color: "#cdd6f4"; background: Rectangle { color: "#313244"; radius: 4 } }
            Button { text: "Set"; onClicked: mainWindow.writeGameState("actors.1.hp", parseInt(hpField.text)) }
        }
        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }
        Button {
            Layout.fillWidth: true; text: "One-Click Victory"
            onClicked: mainWindow.triggerBattleVictory()
            background: Rectangle { color: "#f9e2af"; radius: 4 }
            contentItem: Text { text: "One-Click Victory"; color: "#1e1e2e"; horizontalAlignment: Text.AlignHCenter; font.bold: true }
        }
        Rectangle { Layout.fillHeight: true }
    }
}
