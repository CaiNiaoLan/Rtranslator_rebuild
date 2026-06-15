import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root
    width: 1280; height: 800; visible: true; title: "RTranslator"
    color: "#1e1e2e"; minimumWidth: 900; minimumHeight: 600

    ColumnLayout {
        anchors.fill: parent; spacing: 0
        TopBar { Layout.fillWidth: true; Layout.preferredHeight: 48 }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
            TranslationPanel { Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredWidth: root.width * 0.4 }
            Rectangle { width: 1; Layout.fillHeight: true; color: "#313244" }
            ModificationPanel { Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredWidth: root.width * 0.3 }
            Rectangle { width: 1; Layout.fillHeight: true; color: "#313244" }
            GameStatePanel { Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredWidth: root.width * 0.3 }
        }
        StatusBar { Layout.fillWidth: true; Layout.preferredHeight: 28 }
    }
}
