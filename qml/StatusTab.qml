import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 14

        Text { text: "\uD83D\uDCCA \u6E38\u620F\u72B6\u6001"; font.pixelSize: 16; font.bold: true; color: theme.colors.text }

        Item { Layout.fillWidth: true; Layout.fillHeight: true; visible: !mainWindow.isConnected
            ColumnLayout { anchors.centerIn: parent; spacing: 14
                Text { Layout.alignment: Qt.AlignHCenter; text: "\u2B21"; font.pixelSize: 52; color: theme.colors.textMuted; opacity: 0.3 }
                Text { Layout.alignment: Qt.AlignHCenter; text: "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B\u72B6\u6001"; color: theme.colors.textMuted; font.pixelSize: 13 }
            }
        }

        ColumnLayout { Layout.fillWidth: true; spacing: 12; visible: mainWindow.isConnected

            // Party
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 140; radius: theme.metrics.radius
                color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                ColumnLayout { anchors.fill: parent; anchors.margins: 14; spacing: 6
                    Text { text: "\uD83D\uDC65 \u961F\u4F0D"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }
                    Rectangle { width: parent.width; height: 1; color: theme.colors.border }
                    Flow { Layout.fillWidth: true; spacing: 8
                        Repeater {
                            model: mainWindow.partyStatusModel
                            delegate: Rectangle {
                                width: 160; height: 50; radius: 8
                                color: theme.colors.surfaceAlt; border.color: theme.colors.border; border.width: theme.metrics.borderWidth
                                ColumnLayout { anchors.fill: parent; anchors.margins: 8; spacing: 2
                                    Text { text: modelData.name || ("\u89D2\u8272" + (index + 1)); color: theme.colors.text; font.pixelSize: 11; font.bold: true }
                                    Text { text: "Lv." + (modelData.level || "?") + "  HP " + (modelData.hp || "?") + "/" + (modelData.maxHp || "?"); color: theme.colors.textMuted; font.pixelSize: 10 }
                                }
                            }
                        }
                    }
                }
            }

            // Map Info
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 70; radius: theme.metrics.radius
                color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                RowLayout { anchors.fill: parent; anchors.margins: 14; spacing: 12
                    Text { text: "\uD83D\uDDFA"; font.pixelSize: 24 }
                    ColumnLayout {
                        Text { text: mainWindow.currentMapName || "\u5F53\u524D\u5730\u56FE"; color: theme.colors.text; font.pixelSize: 13; font.bold: true }
                        Text { text: "ID: " + (mainWindow.currentMapId || "-") + "  \u5750\u6807: (" + (mainWindow.playerX || "-") + ", " + (mainWindow.playerY || "-") + ")"; color: theme.colors.textMuted; font.pixelSize: 10 }
                    }
                }
            }

            // Connection
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 60; radius: theme.metrics.radius
                color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                RowLayout { anchors.fill: parent; anchors.margins: 14; spacing: 12
                    Rectangle { width: 10; height: 10; radius: 5; color: theme.colors.success }
                    Text { text: "\u5DF2\u8FDE\u63A5"; color: theme.colors.success; font.pixelSize: 13; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Text { text: mainWindow.translationStats || ""; color: theme.colors.textMuted; font.pixelSize: 11 }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
