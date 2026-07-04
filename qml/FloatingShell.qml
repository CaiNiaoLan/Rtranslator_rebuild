// FloatingShell.qml — 浮动面板风格
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: theme.colors.bg

    TabBar { id: tabBar; visible: false }

    // 背景：铺满的画布
    Item {
        anchors.fill: parent
    }

    // 顶栏：极细工具条
    Rectangle {
        id: topbar
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        height: 36
        color: theme.colors.surface
        border.color: theme.colors.border; border.width: 1
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 8; spacing: 8
            Repeater {
                model: ["\uD83C\uDFE0","\uD83C\uDF10","\uD83D\uDDFA","\uD83D\uDD00","\uD83C\uDFAD","\uD83D\uDCCA"]
                delegate: Button {
                    required property int index
                    required property string modelData
                    text: modelData
                    padding: 4
                    onClicked: tabBar.currentIndex = index
                    background: Rectangle {
                        radius: 6
                        color: tabBar.currentIndex === index ? theme.colors.accent : "transparent"
                    }
                    contentItem: Text {
                        text: parent.text
                        color: tabBar.currentIndex === index ? theme.colors.accentText : theme.colors.text
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                }
            }
            Item { Layout.fillWidth: true }
            ThemeSwitcher { }
        }
    }

    // 主区：浮动面板式
    Rectangle {
        id: dropZone
        anchors.left: parent.left; anchors.right: parent.right
        anchors.top: topbar.bottom; anchors.bottom: parent.bottom
        color: "transparent"

        // 拖放框（左下角的浮动窗口，可拖动）
        Rectangle {
            id: dragPanel
            x: 12; y: parent.height - 140
            width: parent.width - 24
            height: 96
            radius: theme.metrics.radius
            color: theme.colors.surface
            border.color: theme.colors.border; border.width: 1
            opacity: 0.95
            Behavior on opacity { NumberAnimation { duration: 200 } }

            property bool dragActive: false
            property point dragStart: "0,0"

            RowLayout {
                anchors.fill: parent; anchors.margins: 12; spacing: 14
                Text { text: "\uD83D\uDCC2"; font.pixelSize: 26; color: theme.colors.accent }
                ColumnLayout {
                    Layout.fillWidth: true; spacing: 2
                    Text { text: mainWindow.gamePath !== "" ? mainWindow.gamePath : "\u62D6\u62FD Game.exe \u5230\u8FD9\u91CC"; color: theme.colors.text; font.pixelSize: 13 }
                    Text { text: mainWindow.isConnected ? "\u5DF2\u8FDE\u63A5\uFF0C\u53EF\u5F00\u59CB\u4F5C\u5F0A" : "\u672A\u542F\u52A8\u6E38\u620F"; color: theme.colors.textMuted; font.pixelSize: 11 }
                }
                CButton {
                    label: "\u6D4F\u89C8"; onClicked: mainWindow.browseGameFile()
                }
                CButton {
                    label: "\u25B6"; isPrimary: true
                    enabled: mainWindow.gamePath !== "" && !mainWindow.isConnected
                    onClicked: mainWindow.launchGame()
                }
            }

            // drag to move panel
            Drag.active: dragArea.drag.active
            Rectangle {
                anchors.fill: parent
                anchors.margins: -4
                color: "transparent"
                Drag.source: dragPanel
                MouseArea {
                    id: dragArea
                    anchors.right: parent.right
                    anchors.top: parent.top
                    width: 30; height: 22
                    cursorShape: Qt.SizeAllCursor
                    drag.target: parent
                    drag.axis: Drag.YAxis
                    onPressed: parent.parent.z = 1000
                    onReleased: parent.parent.z = 1
                }
            }
        }

        // cheat 浮动面板（右上）
        Rectangle {
            id: cheatPanel
            x: parent.width - width - 12; y: 12
            width: Math.min(360, parent.width * 0.4)
            height: Math.min(220, parent.height * 0.4)
            radius: theme.metrics.radius
            color: theme.colors.surface
            border.color: theme.colors.border; border.width: 1
            opacity: 0.95
            Behavior on opacity { NumberAnimation { duration: 200 } }
            visible: mainWindow.isConnected

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 10; spacing: 6
                RowLayout {
                    Layout.fillWidth: true; spacing: 8
                    Text { text: "\u26A1 \u4E00\u952E\u4F5C\u5F0A"; color: theme.colors.text; font.pixelSize: 12; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Text { text: "\u00D7"; color: theme.colors.textMuted; font.pixelSize: 12 }
                }
                GridLayout {
                    columns: 3; Layout.fillWidth: true; Layout.fillHeight: true
                    columnSpacing: 6; rowSpacing: 6
                    CheatBtn { label: "\u6700\u5927\u91D1\u94B1"; action: "max_gold"; icon: "\uD83D\uDCB0"; color: theme.colors.warning }
                    CheatBtn { label: "\u4E00\u51FB\u5FC5\u6740"; action: "force_win"; icon: "\u2694"; color: theme.colors.danger }
                    CheatBtn { label: "\u5168\u5458\u6062\u590D"; action: "recover_all"; icon: "\uD83D\uDC9A"; color: theme.colors.success }
                    CheatBtn { label: "\u5168\u9053\u5177"; action: "all_items"; icon: "\uD83C\uDF92"; color: theme.colors.success }
                    CheatBtn { label: "\u795E\u6A21\u5F0F"; action: "god_mode"; icon: "\u26A1"; color: theme.colors.accent; isToggle: true }
                    CheatBtn { label: "\u7A7F\u5899"; action: "walk_walls"; icon: "\uD83D\uDC7B"; color: theme.colors.accent; isToggle: true }
                }
                Item { Layout.fillWidth: true }
                CButton {
                    Layout.fillWidth: true; Layout.preferredHeight: 32
                    label: "\u2694 \u4E00\u51FB\u5FC5\u6740"
                    isPrimary: true
                    onClicked: mainWindow.triggerBattleVictory()
                }
            }
        }

        // 真正的 tab 内容：右下主面板
        Rectangle {
            anchors.right: parent.right; anchors.left: parent.left
            anchors.top: parent.top; anchors.bottom: parent.bottom
            anchors.margins: 12
            radius: theme.metrics.radius
            color: theme.colors.surface
            border.color: theme.colors.border; border.width: 1

            StackLayout {
                anchors.fill: parent; anchors.margins: 14
                currentIndex: tabBar.currentIndex
                Item { /* spacer for Home — drag/cheat panels overlap */ }
                TranslationTab {}
                MapTab {}
                VariableTab {}
                EventTab {}
                StatusTab {}
            }
        }
    }
}
