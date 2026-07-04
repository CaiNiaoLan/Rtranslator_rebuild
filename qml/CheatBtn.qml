// CheatBtn.qml — 通用 cheat 按钮（图标 + 文字）
import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    property string label: ""
    property string action: ""
    property string icon: ""
    property color accentColor: theme.colors.accent
    property bool isToggle: false
    property bool active: false

    implicitHeight: theme.metrics.chipHeight
    implicitWidth: labelText.implicitWidth + 36
    radius: height / 2

    color: active ? accentColor
           : (mouseArea.containsMouse ? accentColor : theme.colors.chipBg)
    border.color: active ? accentColor
                : (mouseArea.containsMouse ? accentColor : "transparent")
    border.width: 1
    Behavior on color { ColorAnimation { duration: 120 } }

    Text {
        id: labelText
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: iconItem.width > 0 ? iconItem.width / 2 + 4 : 0
        text: (active && isToggle ? "\u2713 " : "") + root.label
        color: (mouseArea.containsMouse || active) ? "white" : theme.colors.chipText
        font.pixelSize: 11; font.bold: active
    }

    Text {
        id: iconItem
        anchors.right: labelText.left
        anchors.rightMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        text: root.icon
        visible: root.icon.length > 0
        font.pixelSize: 13
        color: (mouseArea.containsMouse || active) ? "white" : theme.colors.chipText
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            mainWindow.cheatAction(root.action)
            if (root.isToggle) root.active = !root.active
            else { root.active = true; flash.restart() }
        }
    }
    Timer { id: flash; interval: 1200; onTriggered: root.active = false }
}
