import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    property string icon: ""
    property string label: ""
    property string defaultValue: "0"
    property color fieldColor: theme.colors.text
    property string writePath: ""
    property int extraWidth: 0
    property Component extraComponent: null

    radius: theme.metrics.radiusSmall
    color: theme.colors.surfaceAlt
    border.color: theme.colors.border
    border.width: theme.metrics.borderWidth

    RowLayout {
        anchors.fill: parent; anchors.margins: 10; spacing: 8
        Text { text: icon + " " + label; color: theme.colors.text; font.pixelSize: 12; Layout.preferredWidth: 70 }
        Loader { Layout.preferredWidth: extraWidth; sourceComponent: extraComponent }
        TextField {
            id: valField; Layout.fillWidth: true
            text: defaultValue; color: fieldColor; font.pixelSize: 13
            background: Rectangle { radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border }
        }
        CButton { label: "修改"; onClicked: { mainWindow.writeGameState(writePath, parseInt(valField.text) || 0) } }
    }
}
