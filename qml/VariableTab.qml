import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 14

        // Switches
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredHeight: parent.height * 0.45
            radius: theme.metrics.radius; color: theme.colors.surface
            border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 6
                Text { text: "\uD83D\uDD18 \u5F00\u5173 (Switches)"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                RowLayout { Layout.fillWidth: true; spacing: 8
                    Text { text: "\u8DF3\u8F6C ID:"; color: theme.colors.textMuted; font.pixelSize: 11 }
                    Rectangle { width: 60; height: 24; radius: 4; color: theme.colors.inputBg; border.color: theme.colors.border; border.width: theme.metrics.borderWidth
                        TextInput { id: switchJumpId; anchors.fill: parent; anchors.margins: 4; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 11; text: "1" }
                    }
                    CButton { label: "\u8DF3\u8F6C"; onClicked: { var idx = parseInt(switchJumpId.text) - 1; if (idx >= 0 && idx < switchList.count) switchList.positionViewAtIndex(idx, ListView.Beginning) } }
                }

                ListView {
                    id: switchList; Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: mainWindow.isConnected ? mainWindow.switchListModel : []
                    delegate: RowLayout {
                        width: switchList.width; height: 30; spacing: 8
                        Text { text: "#" + modelData.id; color: theme.colors.textMuted; font.pixelSize: 10; Layout.preferredWidth: 40 }
                        Rectangle { width: 10; height: 10; radius: 5; color: modelData.value ? theme.colors.success : theme.colors.textMuted }
                        Text { text: modelData.value ? "ON" : "OFF"; color: modelData.value ? theme.colors.success : theme.colors.textMuted; font.pixelSize: 10; Layout.preferredWidth: 30 }
                        CButton { label: "\u5F00"; isSuccess: true; onClicked: mainWindow.toggleSwitch(modelData.id, true) }
                        CButton { label: "\u5173"; isDanger: true; onClicked: mainWindow.toggleSwitch(modelData.id, false) }
                    }
                    Text { anchors.centerIn: parent; text: mainWindow.isConnected ? "" : "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B"; color: theme.colors.textMuted; font.pixelSize: 12; visible: !mainWindow.isConnected }
                }
            }
        }

        // Variables
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredHeight: parent.height * 0.45
            radius: theme.metrics.radius; color: theme.colors.surface
            border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 6
                Text { text: "\uD83D\uDD22 \u53D8\u91CF (Variables)"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                RowLayout { Layout.fillWidth: true; spacing: 8
                    Text { text: "\u8DF3\u8F6C ID:"; color: theme.colors.textMuted; font.pixelSize: 11 }
                    Rectangle { width: 60; height: 24; radius: 4; color: theme.colors.inputBg; border.color: theme.colors.border; border.width: theme.metrics.borderWidth
                        TextInput { id: varJumpId; anchors.fill: parent; anchors.margins: 4; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 11; text: "1" }
                    }
                    CButton { label: "\u8DF3\u8F6C"; onClicked: { var idx = parseInt(varJumpId.text) - 1; if (idx >= 0 && idx < varList.count) varList.positionViewAtIndex(idx, ListView.Beginning) } }
                }

                ListView {
                    id: varList; Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: mainWindow.isConnected ? mainWindow.variableListModel : []
                    delegate: RowLayout {
                        width: varList.width; height: 32; spacing: 8
                        Text { text: "#" + modelData.id; color: theme.colors.textMuted; font.pixelSize: 10; Layout.preferredWidth: 40 }
                        Rectangle { Layout.fillWidth: true; height: 26; radius: 4; color: theme.colors.inputBg; border.color: theme.colors.border; border.width: theme.metrics.borderWidth
                            TextInput {
                                id: varEdit; anchors.fill: parent; anchors.leftMargin: 8
                                verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 12
                                text: modelData.value
                            }
                        }
                        CButton { label: "\u4FEE\u6539"; onClicked: mainWindow.setVariable(modelData.id, parseInt(varEdit.text) || 0) }
                    }
                    Text { anchors.centerIn: parent; text: mainWindow.isConnected ? "" : "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B"; color: theme.colors.textMuted; font.pixelSize: 12; visible: !mainWindow.isConnected }
                }
            }
        }
    }
}
