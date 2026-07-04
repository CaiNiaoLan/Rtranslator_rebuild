import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 16

        // Game Selection
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 110; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            RowLayout { anchors.fill: parent; anchors.margins: 16; spacing: 16
                Text { text: "\uD83C\uDFAE"; font.pixelSize: 36 }
                ColumnLayout { Layout.fillWidth: true; spacing: 8
                    Text { text: "\u9009\u62E9\u6E38\u620F"; font.pixelSize: 14; font.bold: true; color: theme.colors.text }
                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 32; radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border; border.width: theme.metrics.borderWidth
                        TextInput {
                            anchors.fill: parent; anchors.leftMargin: 10; verticalAlignment: Text.AlignVCenter
                            text: mainWindow.gamePath; readOnly: true; color: theme.colors.textMuted; font.pixelSize: 11; clip: true
                        }
                    }
                    RowLayout { spacing: 8
                        CButton { label: "\u6D4F\u89C8"; onClicked: mainWindow.browseGameFile() }
                        CButton { label: "\u25B6 \u542F\u52A8"; isPrimary: true; enabled: mainWindow.gamePath !== "" && !mainWindow.isConnected; onClicked: mainWindow.launchGame() }
                    }
                }
                Rectangle { width: 1; height: 70; color: theme.colors.border }
                ColumnLayout {
                    Text { text: "\u6216\u62D6\u62FD"; color: theme.colors.textMuted; font.pixelSize: 10 }
                    Text { text: "exe \u5230\u7A97\u53E3"; color: theme.colors.textMuted; font.pixelSize: 10 }
                }
            }
        }

        // Quick Cheats
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 10
                Text { text: "\u26A1 \u4E00\u952E\u4F5C\u5F0A"; font.pixelSize: 14; font.bold: true; color: theme.colors.text }

                Flow { Layout.fillWidth: true; Layout.fillHeight: true; spacing: 8
                    CheatBtn { label: "\u6700\u5927\u91D1\u94B1"; action: "max_gold"; icon: "\uD83D\uDCB0"; accentColor: theme.colors.warning }
                    CheatBtn { label: "\u4E00\u51FB\u5FC5\u6740"; action: "force_win"; icon: "\u2694"; accentColor: theme.colors.danger }
                    CheatBtn { label: "\u5168\u5458\u6062\u590D"; action: "recover_all"; icon: "\uD83D\uDC9A"; accentColor: theme.colors.success }
                    CheatBtn { label: "\u5168\u9053\u5177"; action: "all_items"; icon: "\uD83C\uDF92"; accentColor: theme.colors.success }
                    CheatBtn { label: "\u795E\u6A21\u5F0F"; action: "god_mode"; icon: "\u26A1"; accentColor: theme.colors.accent; isToggle: true }
                    CheatBtn { label: "\u7A7F\u5899"; action: "walk_walls"; icon: "\uD83D\uDC7B"; accentColor: theme.colors.accent; isToggle: true }
                    CheatBtn { label: "\u53475\u7EA7"; action: "level_up"; icon: "\u2B50"; accentColor: theme.colors.warning }
                    CheatBtn { label: "\u9003\u8DD1"; action: "force_escape"; icon: "\uD83C\uDFC3"; accentColor: theme.colors.accent }
                }
            }
        }
    }
}
