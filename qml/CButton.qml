import QtQuick 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    property string label: ""
    property string icon: ""
    property bool isPrimary: false
    property bool isDanger: false
    property bool isSuccess: false
    property bool enabled: true

    signal clicked()

    width: labelText.implicitWidth + (icon !== "" ? 24 : 20) + 16
    height: 30; radius: theme.metrics.radiusSmall

    color: {
        if (!enabled) return theme.colors.surfaceAlt
        if (isPrimary) return theme.colors.accent
        if (isDanger) return theme.colors.danger
        if (isSuccess) return theme.colors.success
        return theme.colors.surfaceAlt
    }
    border.color: isPrimary || isDanger || isSuccess ? "transparent" : theme.colors.border
    border.width: theme.metrics.borderWidth
    opacity: enabled ? 1 : 0.4

    RowLayout {
        anchors.fill: parent; anchors.margins: 6; spacing: 4
        Text { visible: icon !== ""; text: icon; font.pixelSize: 13 }
        Text {
            id: labelText
            text: label
            color: enabled ? (isPrimary || isDanger || isSuccess ? "#fff" : theme.colors.text) : theme.colors.textMuted
            font.pixelSize: 11
            font.bold: isPrimary || isDanger || isSuccess
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    MouseArea {
        anchors.fill: parent; cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
        enabled: root.enabled
        onClicked: root.clicked()
    }
}
