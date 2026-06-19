import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root; width: 1320; height: 840; visible: true
    title: "RTranslator"; color: theme.colors.bg
    minimumWidth: 1040; minimumHeight: 680

    DropArea {
        anchors.fill: parent
        onDropped: {
            if (drop.urls.length > 0) {
                var p = drop.urls[0].toString().replace(/^file:\/{2,3}/, "").replace(/\//g, "\\")
                p = decodeURIComponent(p)
                if (p.toLowerCase().endsWith(".exe")) mainWindow.setGamePath(p)
            }
        }
    }

    ColumnLayout { anchors.fill: parent; spacing: 0

        // ---- TOOLBAR ----
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 48
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 14; spacing: 10
                Text { text: "RTranslator"; color: theme.colors.accent; font.pixelSize: 17; font.bold: true }
                Item { Layout.fillWidth: true }
                CButton { label: "浏览"; onClicked: mainWindow.browseGameFile() }
                CButton { label: "启动"; isPrimary: true; enabled: mainWindow.gamePath !== "" && !mainWindow.isConnected; onClicked: mainWindow.launchGame() }
                CButton { label: "断开"; isDanger: true; enabled: mainWindow.isConnected; onClicked: mainWindow.detachGame() }
                CButton { label: "\uD83C\uDFA8"; onClicked: theme.cycleTheme() }
            }
        }

        // ---- TAB BAR ----
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            background: Rectangle { color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth }

            Repeater {
                model: ["\uD83C\uDFE0 \u4E3B\u9875", "\uD83C\uDF10 \u7FFB\u8BD1", "\uD83D\uDDFA \u5730\u56FE", "\uD83D\uDD00 \u53D8\u91CF", "\uD83C\uDFAD \u4E8B\u4EF6", "\uD83D\uDCCA \u72B6\u6001"]
                TabButton {
                    text: modelData
                    width: implicitWidth + 20
                    font.pixelSize: 12
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? theme.colors.accent : theme.colors.textMuted
                        font: parent.font
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.checked ? theme.colors.surfaceAlt : "transparent"
                        radius: 0
                        Rectangle {
                            width: parent.width; height: 3; anchors.bottom: parent.bottom
                            color: parent.parent.checked ? theme.colors.accent : "transparent"
                        }
                    }
                }
            }
        }

        // ---- TAB CONTENT ----
        StackLayout {
            id: tabStack
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            HomeTab {}
            TranslationTab {}
            MapTab {}
            VariableTab {}
            EventTab {}
            StatusTab {}
        }

        // ---- BOTTOM BAR ----
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 26
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            RowLayout { anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 10
                Rectangle { width: 6; height: 6; radius: 3; color: mainWindow.isConnected ? theme.colors.success : theme.colors.textMuted
                    Rectangle { anchors.centerIn: parent; width: 12; height: 12; radius: 6; color: "transparent"; border.color: mainWindow.isConnected ? theme.colors.success : "transparent"
                        visible: mainWindow.isConnected
                        NumberAnimation on opacity { from: 0.8; to: 0; duration: 1500; loops: Animation.Infinite; running: mainWindow.isConnected }
                    }
                }
                Text { text: mainWindow.isConnected ? "\u5DF2\u8FDE\u63A5" : "\u5C31\u7EEA"; color: mainWindow.isConnected ? theme.colors.success : theme.colors.textMuted; font.pixelSize: 11 }
                Item { Layout.fillWidth: true }
                Text { text: mainWindow.gamePath !== "" ? mainWindow.gamePath : "\u672A\u9009\u62E9\u6E38\u620F"; color: theme.colors.textMuted; font.pixelSize: 10; elide: Text.ElideMiddle; Layout.maximumWidth: 300 }
            }
        }
    }
}
