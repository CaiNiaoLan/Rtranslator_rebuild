import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: "#1e1e2e"
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 10; spacing: 10
        Text { text: "Translation"; color: "#89b4fa"; font.pixelSize: 14; font.bold: true }
        RowLayout {
            Button { text: "Extract Text"; onClicked: mainWindow.extractText() }
            Button { text: "Load Translation JSON"; onClicked: {} }
        }
        Text { text: mainWindow.translationStats; color: "#a6adc8"; font.pixelSize: 11 }
        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }
        TextField {
            id: searchField; Layout.fillWidth: true; placeholderText: "Search strings..."
            color: "#cdd6f4"; background: Rectangle { color: "#313244"; radius: 4 }
        }
        ListView {
            id: entryList; Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            model: ListModel { id: entryModel }
            delegate: Rectangle {
                width: entryList.width; height: 36; color: index % 2 === 0 ? "#1e1e2e" : "#181825"; border.color: "#313244"
                RowLayout {
                    anchors.fill: parent; anchors.margins: 4; spacing: 6
                    Text { Layout.preferredWidth: 80; text: model.source || ""; color: "#6c7086"; font.pixelSize: 10; elide: Text.ElideRight }
                    Text { Layout.fillWidth: true; text: model.original || ""; color: "#cdd6f4"; font.pixelSize: 11; elide: Text.ElideRight }
                    Text { Layout.preferredWidth: 100; text: model.translation || ""; color: "#a6e3a1"; font.pixelSize: 11; elide: Text.ElideRight }
                }
            }
        }
    }
    Connections {
        target: mainWindow
        function onExtractionComplete(entries) { entryModel.clear(); for (var i = 0; i < entries.length; i++) entryModel.append(entries[i]) }
    }
}
