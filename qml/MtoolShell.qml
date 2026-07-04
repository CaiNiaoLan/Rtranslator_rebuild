// MtoolShell.qml — 直接复刻 mtool 风格：标题栏 + 顶部 pill tabs + 主体内容
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    color: theme.colors.bg

    Component {
        id: tabImpl
        Item {
            property alias content: stack.children
        }
    }

    ColumnLayout { anchors.fill: parent; spacing: 0

        // 顶部标题 + 主题切换
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: theme.metrics.chipHeight + 12
            color: theme.colors.surface
            RowLayout {
                anchors.fill: parent; anchors.margins: 8
                spacing: 8
                Text { text: "RTranslator"; color: theme.colors.accent; font.bold: true }
                Item { Layout.fillWidth: true }
                ThemeSwitcher { }
            }
        }

        // pill tabs（仿 mtool）
        Rectangle {
            id: pillRow
            Layout.fillWidth: true
            Layout.preferredHeight: theme.metrics.chipHeight + 14
            color: theme.colors.bg
            RowLayout {
                anchors.fill: parent; anchors.margins: 8
                spacing: 6
                Repeater {
                    model: ["\u4E3B\u9875","\u7FFB\u8BD1","\u5730\u56FE","\u53D8\u91CF","\u4E8B\u4EF6","\u72B6\u6001"]
                    delegate: Rectangle {
                        required property int index
                        required property string modelData
                        radius: height/2; height: theme.metrics.chipHeight
                        implicitWidth: pillLabel.implicitWidth + 22
                        color: tabBar.currentIndex === index ? theme.colors.accent : theme.colors.chipBg
                        border.color: tabBar.currentIndex === index ? theme.colors.accent : "transparent"
                        Label {
                            id: pillLabel; anchors.centerIn: parent
                            text: parent.modelData
                            color: tabBar.currentIndex === index ? theme.colors.accentText : theme.colors.chipText
                            font.pixelSize: 12; font.bold: tabBar.currentIndex === index
                        }
                        MouseArea { anchors.fill: parent; onClicked: tabBar.currentIndex = index }
                    }
                }
                Item { Layout.fillWidth: true }
            }
        }

        TabBar { id: tabBar; visible: false; currentIndex: 0 }

        // 内容
        StackLayout {
            id: stack
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: tabBar.currentIndex
            HomeTabMtool {}
            TranslationTab {}
            MapTab {}
            VariableTab {}
            EventTab {}
            StatusTab {}
        }
    }
}
