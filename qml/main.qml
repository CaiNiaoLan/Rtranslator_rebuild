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
                var p = drop.urls[0].toString().replace(/^file:\/{2,3}/, "").replace(/\//g, "\\"); p = decodeURIComponent(p)
                if (p.toLowerCase().endsWith(".exe")) mainWindow.setGamePath(p)
            }
        }
    }

    ColumnLayout { anchors.fill: parent; spacing: 0

        // TOP BAR
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 48
            color: theme.colors.surface; border.color: theme.colors.border

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

        // CONTENT
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true
            Layout.margins: 10; spacing: 10

            // LEFT - Translation
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.35; Layout.minimumWidth: 300
                radius: theme.metrics.radius; color: theme.colors.surface
                border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 12

                    Text { text: "翻译管理"; font.pixelSize: 16; font.bold: true; color: theme.colors.text }

                    // Load JSON
                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 70; radius: theme.metrics.radiusSmall
                        color: theme.colors.surfaceAlt; border.color: theme.colors.border
                        ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 6
                            Text { text: "加载翻译文件"; color: theme.colors.textMuted; font.pixelSize: 11 }
                            RowLayout { spacing: 6
                                Rectangle { Layout.fillWidth: true; height: 28; radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border
                                    TextInput {
                                        id: transField; anchors.fill: parent; anchors.leftMargin: 10
                                        verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 12; clip: true
                                        Text { anchors.fill: parent; anchors.leftMargin: 10; verticalAlignment: Text.AlignVCenter; text: "翻译文件 .json 路径..."; color: theme.colors.textMuted; font.pixelSize: 12; visible: transField.text === "" }
                                    }
                                }
                                CButton { label: "浏览"; onClicked: { var p = mainWindow.browseTranslationFile(); if (p !== "") transField.text = p } }
                                CButton { label: "载入"; isPrimary: true; onClicked: { if (transField.text !== "") mainWindow.loadTranslationFile(transField.text) } }
                            }
                        }
                    }

                    // Export
                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 46; radius: theme.metrics.radiusSmall
                        color: theme.colors.surfaceAlt; border.color: theme.colors.border
                        RowLayout { anchors.fill: parent; anchors.margins: 12; spacing: 8
                            Text { text: mainWindow.translationStats || "未提取"; color: theme.colors.textMuted; font.pixelSize: 12; Layout.fillWidth: true }
                            CButton { label: "导出原文"; onClicked: mainWindow.exportOriginalText() }
                        }
                    }

                    // Status area
                    Rectangle { Layout.fillWidth: true; Layout.fillHeight: true; radius: theme.metrics.radiusSmall; color: theme.colors.surfaceAlt; border.color: theme.colors.border
                        ColumnLayout { anchors.centerIn: parent; spacing: 10
                            Text { Layout.alignment: Qt.AlignHCenter; text: "RTranslator"; font.pixelSize: 14; color: theme.colors.textMuted; font.bold: true }
                            Text { Layout.alignment: Qt.AlignHCenter; text: "选择游戏 → 加载翻译 → 启动"; font.pixelSize: 11; color: theme.colors.textMuted }
                        }
                    }
                }
            }

            // CENTER - Modification
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.35; Layout.minimumWidth: 280
                radius: theme.metrics.radius; color: theme.colors.surface
                border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                Flickable {
                    anchors.fill: parent; anchors.margins: 4; clip: true
                    contentHeight: modColumn.implicitHeight + 32
                    ScrollBar.vertical: ScrollBar {}

                    ColumnLayout {
                        id: modColumn; width: parent.width - 8; anchors.margins: 16; spacing: 8

                        Text { text: "游戏修改"; font.pixelSize: 16; font.bold: true; color: theme.colors.text }

                        // Gold
                        SectionCard {
                            Layout.fillWidth: true; Layout.preferredHeight: 38
                            icon: "\uD83D\uDCB0"; label: "金币"
                            defaultValue: "0"; fieldColor: theme.colors.warning
                            writePath: "party.gold"
                        }

                        // Switch
                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 44; radius: theme.metrics.radiusSmall
                            color: theme.colors.surfaceAlt; border.color: theme.colors.border
                            RowLayout { anchors.fill: parent; anchors.margins: 10; spacing: 8
                                Text { text: "\uD83D\uDD18 开关"; color: theme.colors.text; font.pixelSize: 12; Layout.preferredWidth: 70 }
                                Rectangle { width: 44; height: 26; radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border
                                    TextInput { id: swId; anchors.fill: parent; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 12; text: "1" }
                                }
                                CButton { label: "开"; isSuccess: true; onClicked: mainWindow.writeGameState("switches." + swId.text, true) }
                                CButton { label: "关"; isDanger: true; onClicked: mainWindow.writeGameState("switches." + swId.text, false) }
                            }
                        }

                        // Variable
                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 42; radius: theme.metrics.radiusSmall
                            color: theme.colors.surfaceAlt; border.color: theme.colors.border
                            RowLayout { anchors.fill: parent; anchors.margins: 10; spacing: 8
                                Text { text: "\uD83D\uDD22 变量"; color: theme.colors.text; font.pixelSize: 12; Layout.preferredWidth: 70 }
                                Rectangle { width: 44; height: 26; radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border
                                    TextInput { id: varId; anchors.fill: parent; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: theme.colors.textMuted; font.pixelSize: 11; text: "1" }
                                }
                                TextField {
                                    id: varVal; Layout.fillWidth: true; text: "0"; color: theme.colors.text; font.pixelSize: 13
                                    background: Rectangle { radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border }
                                }
                                CButton { label: "修改"; onClicked: mainWindow.writeGameState("variables." + varId.text, parseInt(varVal.text) || 0) }
                            }
                        }

                        // Actor
                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 110; radius: theme.metrics.radiusSmall
                            color: theme.colors.surfaceAlt; border.color: theme.colors.border
                            ColumnLayout { anchors.fill: parent; anchors.margins: 10; spacing: 4
                                RowLayout {
                                    Text { text: "\uD83D\uDC64 角色"; color: theme.colors.text; font.pixelSize: 12; font.bold: true; Layout.fillWidth: true }
                                    Text { text: "ID"; color: theme.colors.textMuted; font.pixelSize: 11 }
                                    Rectangle { width: 44; height: 22; radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border
                                        TextInput { id: actId; anchors.fill: parent; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 12; text: "1" }
                                    }
                                }
                                GridLayout { columns: 3; columnSpacing: 6; rowSpacing: 2; Layout.fillWidth: true
                                    Text { text: "\u2764 生命"; color: theme.colors.danger; font.pixelSize: 12 }
                                    TextField { id: hpField; Layout.fillWidth: true; text: "0"; color: theme.colors.danger; font.pixelSize: 12; background: Rectangle { radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border } }
                                    CButton { label: "修改"; onClicked: mainWindow.writeGameState("actors." + actId.text + ".hp", parseInt(hpField.text) || 0) }

                                    Text { text: "\uD83D\uDC99 魔力"; color: theme.colors.accent; font.pixelSize: 12 }
                                    TextField { id: mpField; Layout.fillWidth: true; text: "0"; color: theme.colors.accent; font.pixelSize: 12; background: Rectangle { radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border } }
                                    CButton { label: "修改"; onClicked: mainWindow.writeGameState("actors." + actId.text + ".mp", parseInt(mpField.text) || 0) }

                                    Text { text: "\u2B50 等级"; color: theme.colors.warning; font.pixelSize: 12 }
                                    TextField { id: lvField; Layout.fillWidth: true; text: "1"; color: theme.colors.warning; font.pixelSize: 12; background: Rectangle { radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border } }
                                    CButton { label: "修改"; onClicked: mainWindow.writeGameState("actors." + actId.text + ".level", parseInt(lvField.text) || 1) }
                                }
                            }
                        }

                        // Quick cheats
                        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 76; radius: theme.metrics.radiusSmall; color: theme.colors.surfaceAlt; border.color: theme.colors.border
                            ColumnLayout { anchors.fill: parent; anchors.margins: 10; spacing: 6
                                Text { text: "快捷功能"; color: theme.colors.textMuted; font.pixelSize: 11 }
                                RowLayout { spacing: 4
                                    CButton { label: "全员恢复"; onClicked: mainWindow.cheatAction("recover_all") }
                                    CButton { label: "最大金钱"; onClicked: mainWindow.cheatAction("max_gold") }
                                    CButton { label: "全道具"; onClicked: mainWindow.cheatAction("all_items") }
                                    CButton { label: "神模式"; onClicked: mainWindow.cheatAction("god_mode") }
                                    Item { Layout.fillWidth: true }
                                }
                                RowLayout { spacing: 4
                                    CButton { label: "逃跑"; onClicked: mainWindow.cheatAction("force_escape") }
                                    CButton { label: "穿墙"; onClicked: mainWindow.cheatAction("walk_walls") }
                                    CButton { label: "升5级"; onClicked: mainWindow.cheatAction("level_up") }
                                    Item { Layout.fillWidth: true }
                                }
                            }
                        }

                        // Battle
                        CButton { Layout.fillWidth: true; Layout.preferredHeight: 38; isPrimary: true; label: "\u2694 一击必杀"; onClicked: mainWindow.triggerBattleVictory() }
                        Item { Layout.preferredHeight: 10 }
                    }
                }
            }

            // RIGHT - Game State
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.30; Layout.minimumWidth: 220
                radius: theme.metrics.radius; color: theme.colors.surface
                border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 12

                    RowLayout {
                        Text { text: "游戏状态"; font.pixelSize: 16; font.bold: true; color: theme.colors.text; Layout.fillWidth: true }
                        Text { text: mainWindow.isConnected ? "实时" : "离线"; color: mainWindow.isConnected ? theme.colors.success : theme.colors.textMuted; font.pixelSize: 11 }
                    }

                    Item { Layout.fillWidth: true; Layout.fillHeight: true; visible: !mainWindow.isConnected
                        ColumnLayout { anchors.centerIn: parent; spacing: 14
                            Text { Layout.alignment: Qt.AlignHCenter; text: "\u2B21"; font.pixelSize: 52; color: theme.colors.textMuted; opacity: 0.3 }
                            Text { Layout.alignment: Qt.AlignHCenter; text: "启动游戏后查看状态"; color: theme.colors.textMuted; font.pixelSize: 13 }
                        }
                    }

                    ColumnLayout { Layout.fillWidth: true; spacing: 8; visible: mainWindow.isConnected
                        Repeater {
                            model: [
                                { icon: "\uD83D\uDC65", label: "队伍", value: "在线" },
                                { icon: "\uD83D\uDD18", label: "开关", value: "就绪" },
                                { icon: "\uD83D\uDD22", label: "变量", value: "就绪" },
                                { icon: "\uD83D\uDDFA", label: "地图", value: "-" }
                            ]
                            delegate: Rectangle {
                                Layout.fillWidth: true; height: 42; radius: theme.metrics.radiusSmall
                                color: theme.colors.surfaceAlt; border.color: theme.colors.border
                                RowLayout { anchors.fill: parent; anchors.margins: 10; spacing: 10
                                    Text { text: modelData.icon; font.pixelSize: 16 }
                                    Text { text: modelData.label; color: theme.colors.text; font.pixelSize: 12; Layout.preferredWidth: 50 }
                                    Item { Layout.fillWidth: true }
                                    Text { text: modelData.value; color: theme.colors.textMuted; font.pixelSize: 11 }
                                }
                            }
                        }
                        Item { Layout.fillHeight: true }
                    }
                }
            }
        }

        // STATUS BAR
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 26
            color: theme.colors.surface; border.color: theme.colors.border

            RowLayout { anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 10
                Rectangle { width: 6; height: 6; radius: 3; color: mainWindow.isConnected ? theme.colors.success : theme.colors.textMuted
                    Rectangle { anchors.centerIn: parent; width: 12; height: 12; radius: 6; color: "transparent"; border.color: mainWindow.isConnected ? theme.colors.success : "transparent"
                        visible: mainWindow.isConnected
                        NumberAnimation on opacity { from: 0.8; to: 0; duration: 1500; loops: Animation.Infinite; running: mainWindow.isConnected }
                    }
                }
                Text { text: mainWindow.isConnected ? "已连接" : "就绪"; color: mainWindow.isConnected ? theme.colors.success : theme.colors.textMuted; font.pixelSize: 11 }
                Item { Layout.fillWidth: true }
                Text { text: mainWindow.gamePath !== "" ? mainWindow.gamePath : "未选择游戏"; color: theme.colors.textMuted; font.pixelSize: 10; elide: Text.ElideMiddle; Layout.maximumWidth: 300 }
            }
        }
    }
}
