// ThemeSwitcher.qml — 一键切换布局 / 颜色变体
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    implicitHeight: 28

    Rectangle {
        anchors.fill: parent
        radius: parent.height/2
        color: theme.colors.chipBg
        border.color: theme.colors.border; border.width: 1

        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 4; anchors.rightMargin: 4; spacing: 2

            ToolButton {
                text: "\u4E09\u578B\u5E03\u5C40"
                font.pixelSize: 11
                onClicked: layoutMenu.open()
                Menu {
                    id: layoutMenu
                    Repeater {
                        model: theme.layouts
                        delegate: MenuItem {
                            required property int index
                            required property string modelData
                            text: theme.variantsOf(modelData)
                            onTriggered: theme.setLayout(modelData)
                        }
                    }
                }
            }

            Rectangle { Layout.fillHeight: true; width: 1; color: theme.colors.border }

            ToolButton {
                text: "\u989C\u8272"
                font.pixelSize: 11
                onClicked: variantMenu.open()
                Menu {
                    id: variantMenu
                    Repeater {
                        model: theme.variantsOf(theme.layoutName)
                        delegate: MenuItem {
                            required property int index
                            required property string modelData
                            text: modelData
                            onTriggered: theme.setVariant(modelData)
                        }
                    }
                }
            }

            ToolButton {
                text: "\u21BB"
                font.pixelSize: 12
                onClicked: theme.cycleTheme()
            }
        }
    }
}
