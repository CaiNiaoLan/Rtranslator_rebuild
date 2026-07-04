// DrawerShell.qml — 左侧抽屉导航 + 右侧大画布
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: shell
    color: theme.colors.bg

    property int drawerWidth: 220
    property int iconsWidth: 60

    // 左侧抽屉（含可折叠迷你态）
    Rectangle {
        id: drawer
        anchors.left: parent.left; anchors.top: parent.top; anchors.bottom: parent.bottom
        width: shell.drawerWidth
        color: theme.colors.surface
        border.color: theme.colors.border; border.width: 1

        ColumnLayout {
            anchors.fill: parent; spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                color: "transparent"
                RowLayout {
                    anchors.fill: parent; anchors.margins: 12; spacing: 10
                    Text { text: "\uD83C\uDFA8"; font.pixelSize: 22 }
                    Text {
                        text: "RTranslator"
                        color: theme.colors.text
                        font.pixelSize: 15; font.bold: true
                        visible: true
                    }
                    Item { Layout.fillWidth: true }
                }
            }

            Rectangle { Layout.fillWidth: true; height: 1; color: theme.colors.border }

            ListView {
                Layout.fillWidth: true; Layout.fillHeight: true
                model: ListModel {
                    ListElement { label: "\u4E3B\u9875"; icon: "\uD83C\uDFE0" }
                    ListElement { label: "\u7FFB\u8BD1"; icon: "\uD83C\uDF10" }
                    ListElement { label: "\u5730\u56FE"; icon: "\uD83D\uDDFA" }
                    ListElement { label: "\u53D8\u91CF"; icon: "\uD83D\uDD00" }
                    ListElement { label: "\u4E8B\u4EF6"; icon: "\uD83C\uDFAD" }
                    ListElement { label: "\u72B6\u6001"; icon: "\uD83D\uDCCA" }
                }
                delegate: ItemDelegate {
                    text: ""  // suppress default
                    width: ListView.view.width
                    height: 44
                    Rectangle {
                        anchors.fill: parent; anchors.margins: 6
                        radius: 8
                        color: tabBar.currentIndex === index ? theme.colors.chipBg : "transparent"
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 14; spacing: 12
                            Text { text: model.icon; font.pixelSize: 18 }
                            Text {
                                text: model.label
                                color: tabBar.currentIndex === index ? theme.colors.accent : theme.colors.text
                                font.pixelSize: 13; font.bold: tabBar.currentIndex === index
                            }
                            Item { Layout.fillWidth: true }
                        }
                    }
                    onClicked: tabBar.currentIndex = index
                }
            }

            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: theme.colors.border }

            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 48
                color: "transparent"
                RowLayout {
                    anchors.fill: parent; anchors.margins: 12; spacing: 10
                    ThemeSwitcher { }
                    Item { Layout.fillWidth: true }
                }
            }
        }
    }

    // 内容区
    Item {
        anchors.left: drawer.right; anchors.right: parent.right; anchors.top: parent.top; anchors.bottom: parent.bottom
        TabBar { id: tabBar; visible: false }

        StackLayout {
            anchors.fill: parent; anchors.margins: 20
            currentIndex: tabBar.currentIndex
            // Side-Drawer 用更"大画布"风格的主页
            HomeTabDrawer {}
            TranslationTab {}
            MapTab {}
            VariableTab {}
            EventTab {}
            StatusTab {}
        }
    }
}
