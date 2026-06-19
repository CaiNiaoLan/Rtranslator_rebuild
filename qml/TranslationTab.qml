import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 14

        // Card 1: Load Translation
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 100; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 8
                Text { text: "\uD83D\uDCC2 \u52A0\u8F7D\u7FFB\u8BD1\u6587\u4EF6"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }
                RowLayout { spacing: 8
                    Rectangle { Layout.fillWidth: true; height: 32; radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border; border.width: theme.metrics.borderWidth
                        TextInput {
                            id: transField; anchors.fill: parent; anchors.leftMargin: 10
                            verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 12; clip: true
                            Text { anchors.fill: parent; anchors.leftMargin: 10; verticalAlignment: Text.AlignVCenter; text: "\u7FFB\u8BD1\u6587\u4EF6 .json \u8DEF\u5F84..."; color: theme.colors.textMuted; font.pixelSize: 12; visible: transField.text === "" }
                        }
                    }
                    CButton { label: "\u6D4F\u89C8"; onClicked: { var p = mainWindow.browseTranslationFile(); if (p !== "") transField.text = p } }
                    CButton { label: "\u8F7D\u5165"; isPrimary: true; onClicked: { if (transField.text !== "") mainWindow.loadTranslationFile(transField.text) } }
                }
                RowLayout { spacing: 6
                    Text { text: mainWindow.translationStats; color: theme.colors.textMuted; font.pixelSize: 11; Layout.fillWidth: true }
                    Item { Layout.fillWidth: true }
                }
            }
        }

        // Card 2: Extract Original
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 80; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            RowLayout { anchors.fill: parent; anchors.margins: 16; spacing: 12
                ColumnLayout { Layout.fillWidth: true; spacing: 4
                    Text { text: "\uD83D\uDCDD \u63D0\u53D6\u539F\u6587"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }
                    Text { text: mainWindow.translationStats !== "" ? mainWindow.translationStats : "\u4ECE\u6E38\u620F\u6570\u636E\u6587\u4EF6\u4E2D\u63D0\u53D6\u6240\u6709\u53EF\u7FFB\u8BD1\u6587\u5B57"; color: theme.colors.textMuted; font.pixelSize: 11 }
                }
                CButton { label: "\u63D0\u53D6"; onClicked: mainWindow.extractText() }
            }
        }

        // Card 3: Export
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            RowLayout { anchors.fill: parent; anchors.margins: 16; spacing: 12
                ColumnLayout { Layout.fillWidth: true; spacing: 4
                    Text { text: "\uD83D\uDCE4 \u5BFC\u51FA\u539F\u6587"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }
                    Text { text: "\u5C06\u63D0\u53D6\u7684\u539F\u6587\u5BFC\u51FA\u4E3A JSON \u6A21\u677F\u6587\u4EF6"; color: theme.colors.textMuted; font.pixelSize: 11 }
                }
                CButton { label: "\u5BFC\u51FA\u539F\u6587 JSON"; onClicked: mainWindow.exportOriginalText() }
            }
        }
    }
}
