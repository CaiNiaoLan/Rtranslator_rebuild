import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 14

        // Current Map
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 80; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.centerIn: parent; spacing: 4
                RowLayout { Layout.alignment: Qt.AlignHCenter; spacing: 8
                    Text { text: "\uD83D\uDCCD"; font.pixelSize: 20 }
                    Text { text: mainWindow.isConnected ? (mainWindow.currentMapInfo ? mainWindow.currentMapInfo.name || "\u5F53\u524D\u5730\u56FE" : "\u5F53\u524D\u5730\u56FE") : "\u672A\u8FDE\u63A5"; font.pixelSize: 14; font.bold: true; color: theme.colors.text }
                }
                Text { text: mainWindow.isConnected ? ("ID: " + ((mainWindow.currentMapInfo && mainWindow.currentMapInfo.id) ? mainWindow.currentMapInfo.id : "-")) : "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B"; color: theme.colors.textMuted; font.pixelSize: 11 }
            }
        }

        // Map List
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 8
                Text { text: "\uD83D\uDDFA \u5730\u56FE\u4F20\u9001"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                ListView {
                    id: mapList
                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: mainWindow.mapListModel

                    delegate: Rectangle {
                        width: mapList.width; height: 36; radius: 6
                        color: index % 2 === 0 ? theme.colors.surfaceAlt : "transparent"

                        RowLayout { anchors.fill: parent; anchors.margins: 8; spacing: 10
                            Text { text: modelData; color: theme.colors.text; font.pixelSize: 12; Layout.fillWidth: true; elide: Text.ElideRight }
                            CButton { label: "\u4F20\u9001"; onClicked: mainWindow.teleportToMap(index + 1) }
                        }
                    }

                    Text { anchors.centerIn: parent; text: mainWindow.isConnected ? "\u65E0\u5730\u56FE\u6570\u636E" : "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B\u5730\u56FE\u5217\u8868"; color: theme.colors.textMuted; font.pixelSize: 12; visible: mapList.count === 0 }
                }
            }
        }
    }
}
