// HomeTabMtool.qml — 完全复刻 MTool 主页：超大虚线拖放框 + 密集 chip 控件
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Flickable {
    contentWidth: width
    contentHeight: contentColumn.implicitHeight + 24
    clip: true
    ColumnLayout {
        id: contentColumn
        anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
        spacing: 14

    // 大虚线拖放框
    Rectangle {
        Layout.fillWidth: true; Layout.preferredHeight: theme.metrics.dropHeight + 24
        radius: theme.metrics.radius + 4
        color: theme.colors.dropBg
        border.color: theme.colors.dropBorder; border.width: 2
        border.pixelWidth: 2
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 12; spacing: 8
            Text {
                Layout.alignment: Qt.AlignHCenter
                text: mainWindow.gamePath !== ""
                      ? "\u2714  " + mainWindow.gamePath
                      : "\u62D6\u62FD\u6216\u70B9\u51FB\u8FD9\u91CC\u9009\u62E9\u4E00\u4E2A\u6E38\u620F\u4E3B\u7A0B\u5E8F\u6587\u4EF6 (\u4F8B\u5982 Game.exe) \u6765\u68C0\u6D4B\u6E38\u620F\u5F15\u64CE\u3002"
                color: theme.colors.text
                font.pixelSize: 14
                wrapMode: Text.Wrap
                Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter
            }
            CButton {
                Layout.alignment: Qt.AlignHCenter
                label: "\u6D4F\u89C8"
                onClicked: mainWindow.browseGameFile()
            }
        }
    }

    // 游戏引擎行
    RowLayout {
        Layout.fillWidth: true; spacing: 8
        Text { text: "\u6E38\u620F\u5F15\u64CE\uFF1A"; color: theme.colors.text; font.pixelSize: 12 }
        ComboBox { Layout.fillWidth: true }
        CButton { label: "\u8FD0\u884C\u65F6\u5B89\u88C5\u5668" }
    }

    // 启动行（密集 chip）
    Rectangle {
        Layout.fillWidth: true; Layout.preferredHeight: 130
        radius: theme.metrics.radius; color: theme.colors.surface
        border.color: theme.colors.border; border.width: 1
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 10; spacing: 6
            RowLayout { spacing: 6; Layout.fillWidth: true
                CButton { label: "\u542F\u52A8\u6E38\u620F"; isPrimary: true; enabled: mainWindow.gamePath !== "" && !mainWindow.isConnected; onClicked: mainWindow.launchGame() }
                CheckBox { text: "\u4E3B\u52A8\u6CE8\u5165"; checked: true }
                CheckBox { text: "\u751F\u6210\u542F\u52A8\u5668" }
                CheckBox { text: "\u88AB\u52A8\u6CE8\u5165(\u4FEE\u590D)" }
                CheckBox { text: "\u6CE8\u5165\u81F3\u6B63\u5728\u8FD0\u884C\u7684\u6E38\u620F" }
                CheckBox { text: "\u590D\u5236\u5DE5\u5177\u81F3\u6E38\u620F" }
                CheckBox { text: "\u4F7F\u7528\u5DE5\u5177\u7684 NW.js" }
            }
            RowLayout { spacing: 6
                CButton { label: "\u542F\u52A8\u4E0A\u6B21\u73A9\u8FC7\u7684\u6E38\u620F" }
                CButton { label: "\u6253\u5F00\u7F51\u7AD9\u4E0A\u7684 RPG Maker \u6E38\u620F" }
            }
        }
    }

    // cheat 标题
    Text {
        text: "\u4E00\u952E\u4F5C\u5F0A (Cheats)"; color: theme.colors.text; font.bold: true; font.pixelSize: 13
    }

    // cheat 密集网格
    Rectangle {
        Layout.fillWidth: true; Layout.fillHeight: true
        Layout.minimumHeight: 100
        radius: theme.metrics.radius; color: theme.colors.surface
        border.color: theme.colors.border; border.width: 1
        Flow {
            anchors.fill: parent; anchors.margins: 10; spacing: 6
            CheatBtn { label: "\u6700\u5927\u91D1\u94B1"; action: "max_gold"; icon: "\uD83D\uDCB0"; color: theme.colors.warning }
            CheatBtn { label: "\u4E00\u51FB\u5FC5\u6740"; action: "force_win"; icon: "\u2694"; color: theme.colors.danger }
            CheatBtn { label: "\u5168\u5458\u6062\u590D"; action: "recover_all"; icon: "\uD83D\uDC9A"; color: theme.colors.success }
            CheatBtn { label: "\u5168\u9053\u5177"; action: "all_items"; icon: "\uD83C\uDF92"; color: theme.colors.success }
            CheatBtn { label: "\u795E\u6A21\u5F0F"; action: "god_mode"; icon: "\u26A1"; color: theme.colors.accent; isToggle: true }
            CheatBtn { label: "\u7A7F\u5899"; action: "walk_walls"; icon: "\uD83D\uDC7B"; color: theme.colors.accent; isToggle: true }
            CheatBtn { label: "\u53475\u7EA7"; action: "level_up"; icon: "\u2B50"; color: theme.colors.warning }
            CheatBtn { label: "\u9003\u8DD1"; action: "force_escape"; icon: "\uD83C\uDFC3"; color: theme.colors.accent }
            CheatBtn { label: "\u53EC\u5524\u9610\u4E0A"; action: "recover_all"; icon: "\u269C"; color: theme.colors.accent; isToggle: true }
            CheatBtn { label: "\u9000\u51FA\u6E38\u620F"; action: "force_escape"; icon: "\u23F9"; color: theme.colors.danger }
        }
    }

    RowLayout {
        Layout.fillWidth: true; spacing: 8
        CButton { label: "\u542F\u52A8\u4E0A\u6B21\u73A9\u8FC7\u7684\u6E38\u620F" }
        CButton { label: "\u6253\u5F00\u7F51\u7AD9\u4E0A\u7684 RPG Maker \u6E38\u620F" }
        Item { Layout.fillWidth: true }
        Text { text: mainWindow.translationStats || ""; color: theme.colors.textMuted; font.pixelSize: 11 }
    }
    }
}
