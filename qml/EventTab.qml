import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    RowLayout { anchors.fill: parent; spacing: 0

        // Left: Event List
        Rectangle {
            Layout.preferredWidth: parent.width * 0.35; Layout.fillHeight: true
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 8
                Text { text: "\uD83D\uDCCB \u4E8B\u4EF6\u5217\u8868"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                // Common Events
                Text { text: "\u516C\u5171\u4E8B\u4EF6"; font.pixelSize: 11; font.bold: true; color: theme.colors.textMuted }
                ListView {
                    id: commonList; Layout.fillWidth: true; Layout.preferredHeight: parent.height * 0.4; clip: true
                    model: mainWindow.isConnected ? mainWindow.commonEventListModel : []
                    delegate: Rectangle {
                        width: commonList.width; height: 30; radius: 4
                        color: commonList.currentIndex === index ? theme.colors.surfaceAlt : "transparent"
                        Text { anchors.fill: parent; anchors.margins: 8; text: modelData; color: theme.colors.text; font.pixelSize: 11; verticalAlignment: Text.AlignVCenter }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { commonList.currentIndex = index; mainWindow.selectCommonEvent(index + 1) } }
                    }
                }

                // Map Events
                Text { text: "\u5F53\u524D\u5730\u56FE\u4E8B\u4EF6"; font.pixelSize: 11; font.bold: true; color: theme.colors.textMuted }
                ListView {
                    id: mapEventList; Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: mainWindow.isConnected ? mainWindow.mapEventListModel : []
                    delegate: Rectangle {
                        width: mapEventList.width; height: 30; radius: 4
                        color: mapEventList.currentIndex === index ? theme.colors.surfaceAlt : "transparent"
                        Text { anchors.fill: parent; anchors.margins: 8; text: modelData; color: theme.colors.text; font.pixelSize: 11; verticalAlignment: Text.AlignVCenter }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { mapEventList.currentIndex = index; mainWindow.selectMapEvent(index + 1) } }
                    }
                }
            }
        }

        // Right: Event Detail
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: theme.colors.bg; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 10
                Text { text: "\uD83D\uDD0D \u4E8B\u4EF6\u8BE6\u60C5"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true; radius: theme.metrics.radius
                    color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                    ColumnLayout { anchors.fill: parent; anchors.margins: 14; spacing: 8
                        Text { text: mainWindow.selectedEventName || "\u9009\u62E9\u5DE6\u4FA7\u4E8B\u4EF6\u67E5\u770B\u8BE6\u60C5"; color: theme.colors.text; font.pixelSize: 13; font.bold: true }
                        Rectangle { width: parent.width; height: 1; color: theme.colors.border }

                        Text { text: mainWindow.selectedEventDetail || "\u6CA1\u6709\u9009\u4E2D\u4E8B\u4EF6"; color: theme.colors.textMuted; font.pixelSize: 11; Layout.fillWidth: true; wrapMode: Text.Wrap }

                        CButton {
                            label: "\u6267\u884C\u6B64\u4E8B\u4EF6"
                            isPrimary: true; enabled: mainWindow.isConnected && mainWindow.selectedEventId > 0
                            onClicked: mainWindow.triggerSelectedEvent()
                        }
                        Item { Layout.fillHeight: true }
                    }
                }
            }
        }
    }
}
