// HomeTabDrawer.qml — Side-Drawer 风格：宽画布 + 卡片化 cheat 区
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Flickable {
    contentWidth: width
    contentHeight: col.implicitHeight + 40
    clip: true
    ColumnLayout {
        id: col
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        spacing: theme.metrics.spacing + 6
        anchors.topMargin: theme.metrics.padding

    // 标题 + 全局操作
    RowLayout {
        Layout.fillWidth: true
        Text { text: "\u4E3B\u9875"; font.pixelSize: 22; font.bold: true; color: theme.colors.text }
        Item { Layout.fillWidth: true }
        CButton {
            label: "\u52A0\u8F7D\u7FFB\u8BD1"; onClicked: mainWindow.loadTranslationFile("")
        }
        CButton { label: "\u63D0\u53D6\u539F\u6587"; onClicked: mainWindow.exportOriginalText() }
        CButton { label: "\u542F\u52A8\u6E38\u620F"; isPrimary: true
                 enabled: mainWindow.gamePath !== "" && !mainWindow.isConnected
                 onClicked: mainWindow.launchGame() }
        CButton { label: "\u65AD\u5F00"; isDanger: true; enabled: mainWindow.isConnected
                  onClicked: mainWindow.detachGame() }
    }

    // 游戏选择大卡片
    Rectangle {
        Layout.fillWidth: true; Layout.preferredHeight: 100
        radius: theme.metrics.radius; color: theme.colors.surface
        border.color: theme.colors.border; border.width: 1
        RowLayout {
            anchors.fill: parent; anchors.margins: 16; spacing: 16
            Text { text: "\uD83C\uDFAE"; font.pixelSize: 36 }
            ColumnLayout { Layout.fillWidth: true; spacing: 4
                Text { text: "\u9009\u62E9\u6E38\u620F"; font.bold: true; color: theme.colors.text }
                Text { text: mainWindow.gamePath || "\u8BF7\u9009\u62E9 Game.exe"; color: theme.colors.textMuted; elide: Text.ElideMiddle; Layout.fillWidth: true; font.pixelSize: 12 }
            }
            CButton { label: "\u6D4F\u89C8"; onClicked: mainWindow.browseGameFile() }
        }
    }

    // 三列 cheat 卡片
    Text { text: "\u4E00\u952E\u4F5C\u5F0A"; font.bold: true; font.pixelSize: 14; color: theme.colors.text }

    GridLayout {
        Layout.fillWidth: true
        columns: 4; columnSpacing: theme.metrics.spacing; rowSpacing: theme.metrics.spacing

        component QuickCard: Rectangle {
            property string label: ""
            property string icon: ""
            property color accent: theme.colors.accent
            property string action: ""
            implicitWidth: 160; implicitHeight: 80
            radius: theme.metrics.radius
            color: theme.colors.surface
            border.color: border.color; border.width: 1

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 12; spacing: 6
                Text { text: icon; font.pixelSize: 22; color: accent }
                Text { text: label; color: theme.colors.text; font.pixelSize: 12; font.bold: true }
                Item { Layout.fillHeight: true }
            }
            MouseArea {
                anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                onClicked: mainWindow.cheatAction(action)
            }
        }

        QuickCard { label: "\u4E00\u51FB\u5FC5\u6740"; icon: "\u2694"; accent: theme.colors.danger; action: "force_win" }
        QuickCard { label: "\u6700\u5927\u91D1\u94B1"; icon: "\uD83D\uDCB0"; accent: theme.colors.warning; action: "max_gold" }
        QuickCard { label: "\u5168\u5458\u6062\u590D"; icon: "\uD83D\uDC9A"; accent: theme.colors.success; action: "recover_all" }
        QuickCard { label: "\u5168\u9053\u5177"; icon: "\uD83C\uDF92"; accent: theme.colors.success; action: "all_items" }
        QuickCard { label: "\u795E\u6A21\u5F0F"; icon: "\u26A1"; accent: theme.colors.accent; action: "god_mode" }
        QuickCard { label: "\u7A7F\u5899"; icon: "\uD83D\uDC7B"; accent: theme.colors.accent; action: "walk_walls" }
        QuickCard { label: "\u53475\u7EA7"; icon: "\u2B50"; accent: theme.colors.warning; action: "level_up" }
        QuickCard { label: "\u9003\u8DD1"; icon: "\uD83C\uDFC3"; accent: theme.colors.accent; action: "force_escape" }
    }

    Item { Layout.fillHeight: true }
    }
}
