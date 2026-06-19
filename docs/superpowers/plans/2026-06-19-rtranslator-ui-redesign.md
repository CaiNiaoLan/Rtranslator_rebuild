# RTranslator UI Redesign Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the 3-column QML layout with a 6-tab top-navigation interface, preserving all existing functionality.

**Architecture:** `main.qml` becomes a layout shell with `TabBar` + `StackLayout`. Each tab gets its own QML file. The multi-theme system (ThemeManager with 5 themes) is preserved. CButton.qml and SectionCard.qml are reused. New `Q_INVOKABLE` stubs are added to MainWindow for tab data needs.

**Tech Stack:** Qt 5.15, QML, C++17, MingGW GCC 8.1.0

---

### Task 1: Delete obsolete QML files and update qrc

**Files:**
- Delete: `qml/TopBar.qml`, `qml/StatusBar.qml`, `qml/TranslationPanel.qml`, `qml/ModificationPanel.qml`, `qml/GameStatePanel.qml`, `qml/CheatBtn.qml`
- Modify: `qml/qml.qrc`

- [ ] **Step 1: Remove old QML files**

```powershell
Remove-Item -LiteralPath "E:\Rtranslator_rebuild\qml\TopBar.qml" -Force
Remove-Item -LiteralPath "E:\Rtranslator_rebuild\qml\StatusBar.qml" -Force
Remove-Item -LiteralPath "E:\Rtranslator_rebuild\qml\TranslationPanel.qml" -Force
Remove-Item -LiteralPath "E:\Rtranslator_rebuild\qml\ModificationPanel.qml" -Force
Remove-Item -LiteralPath "E:\Rtranslator_rebuild\qml\GameStatePanel.qml" -Force
Remove-Item -LiteralPath "E:\Rtranslator_rebuild\qml\CheatBtn.qml" -Force
```

- [ ] **Step 2: Update qml.qrc**

Replace the content of `qml/qml.qrc` with:

```xml
<RCC>
    <qresource prefix="/qml">
        <file>main.qml</file>
        <file>CButton.qml</file>
        <file>SectionCard.qml</file>
        <file>HomeTab.qml</file>
        <file>TranslationTab.qml</file>
        <file>MapTab.qml</file>
        <file>VariableTab.qml</file>
        <file>EventTab.qml</file>
        <file>StatusTab.qml</file>
    </qresource>
</RCC>
```

- [ ] **Step 3: Commit**

```bash
git add qml/qml.qrc
git commit -m "chore: remove obsolete QML files, update qrc for tab layout"
```

---

### Task 2: Rewrite main.qml — TabBar + StackLayout shell

**Files:**
- Modify: `qml/main.qml`

- [ ] **Step 1: Replace main.qml content**

Write `qml/main.qml`:

```qml
import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Window {
    id: root; width: 1320; height: 840; visible: true
    title: "RTranslator"; color: theme.colors.bg
    minimumWidth: 1040; minimumHeight: 680

    DropArea {
        anchors.fill: parent
        onDropped: {
            if (drop.urls.length > 0) {
                var p = drop.urls[0].toString().replace(/^file:\/{2,3}/, "").replace(/\//g, "\\")
                p = decodeURIComponent(p)
                if (p.toLowerCase().endsWith(".exe")) mainWindow.setGamePath(p)
            }
        }
    }

    ColumnLayout { anchors.fill: parent; spacing: 0

        // ---- TOOLBAR ----
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 48
            color: theme.colors.surface; border.color: theme.colors.border

            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 14; spacing: 10
                Text { text: "RTranslator"; color: theme.colors.accent; font.pixelSize: 17; font.bold: true }
                Item { Layout.fillWidth: true }
                CButton { label: "浏览"; onClicked: mainWindow.browseGameFile() }
                CButton { label: "启动"; isPrimary: true; enabled: mainWindow.gamePath !== "" && !mainWindow.isConnected; onClicked: mainWindow.launchGame() }
                CButton { label: "断开"; isDanger: true; enabled: mainWindow.isConnected; onClicked: mainWindow.detachGame() }
                CButton { label: "\uD83C\uDFA8"; onClicked: theme.cycleTheme() }
            }
        }

        // ---- TAB BAR ----
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            background: Rectangle { color: theme.colors.surface; border.color: theme.colors.border }

            Repeater {
                model: ["\uD83C\uDFE0 \u4E3B\u9875", "\uD83C\uDF10 \u7FFB\u8BD1", "\uD83D\uDDFA \u5730\u56FE", "\uD83D\uDD00 \u53D8\u91CF", "\uD83C\uDFAD \u4E8B\u4EF6", "\uD83D\uDCCA \u72B6\u6001"]
                TabButton {
                    text: modelData
                    width: implicitWidth + 20
                    font.pixelSize: 12
                    contentItem: Text {
                        text: parent.text
                        color: parent.checked ? theme.colors.accent : theme.colors.textMuted
                        font: parent.font
                        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.checked ? theme.colors.surfaceAlt : "transparent"
                        radius: 0
                        Rectangle {
                            width: parent.width; height: 3; anchors.bottom: parent.bottom
                            color: parent.parent.checked ? theme.colors.accent : "transparent"
                        }
                    }
                }
            }
        }

        // ---- TAB CONTENT ----
        StackLayout {
            id: tabStack
            Layout.fillWidth: true; Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            HomeTab {}
            TranslationTab {}
            MapTab {}
            VariableTab {}
            EventTab {}
            StatusTab {}
        }

        // ---- BOTTOM BAR ----
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 26
            color: theme.colors.surface; border.color: theme.colors.border

            RowLayout { anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 10
                Rectangle { width: 6; height: 6; radius: 3; color: mainWindow.isConnected ? theme.colors.success : theme.colors.textMuted
                    Rectangle { anchors.centerIn: parent; width: 12; height: 12; radius: 6; color: "transparent"; border.color: mainWindow.isConnected ? theme.colors.success : "transparent"
                        visible: mainWindow.isConnected
                        NumberAnimation on opacity { from: 0.8; to: 0; duration: 1500; loops: Animation.Infinite; running: mainWindow.isConnected }
                    }
                }
                Text { text: mainWindow.isConnected ? "\u5DF2\u8FDE\u63A5" : "\u5C31\u7EEA"; color: mainWindow.isConnected ? theme.colors.success : theme.colors.textMuted; font.pixelSize: 11 }
                Item { Layout.fillWidth: true }
                Text { text: mainWindow.gamePath !== "" ? mainWindow.gamePath : "\u672A\u9009\u62E9\u6E38\u620F"; color: theme.colors.textMuted; font.pixelSize: 10; elide: Text.ElideMiddle; Layout.maximumWidth: 300 }
            }
        }
    }
}
```

- [ ] **Step 2: Commit**

```bash
git add qml/main.qml
git commit -m "feat: rewrite main.qml with TabBar + StackLayout shell"
```

---

### Task 3: Create HomeTab.qml — game selection + cheat buttons

**Files:**
- Create: `qml/HomeTab.qml`

- [ ] **Step 1: Write HomeTab.qml**

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 16

        // Game Selection
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 110; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            RowLayout { anchors.fill: parent; anchors.margins: 16; spacing: 16
                Text { text: "\uD83C\uDFAE"; font.pixelSize: 36 }
                ColumnLayout { Layout.fillWidth: true; spacing: 8
                    Text { text: "\u9009\u62E9\u6E38\u620F"; font.pixelSize: 14; font.bold: true; color: theme.colors.text }
                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 32; radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border
                        TextInput {
                            anchors.fill: parent; anchors.leftMargin: 10; verticalAlignment: Text.AlignVCenter
                            text: mainWindow.gamePath; readOnly: true; color: theme.colors.textMuted; font.pixelSize: 11; clip: true
                        }
                    }
                    RowLayout { spacing: 8
                        CButton { label: "\u6D4F\u89C8"; onClicked: mainWindow.browseGameFile() }
                        CButton { label: "\u25B6 \u542F\u52A8"; isPrimary: true; enabled: mainWindow.gamePath !== "" && !mainWindow.isConnected; onClicked: mainWindow.launchGame() }
                    }
                }
                Rectangle { width: 1; height: 70; color: theme.colors.border }
                ColumnLayout {
                    Text { text: "\u6216\u62D6\u62FD"; color: theme.colors.textMuted; font.pixelSize: 10 }
                    Text { text: "exe \u5230\u7A97\u53E3"; color: theme.colors.textMuted; font.pixelSize: 10 }
                }
            }
        }

        // Quick Cheats
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 10
                Text { text: "\u26A1 \u4E00\u952E\u4F5C\u5F0A"; font.pixelSize: 14; font.bold: true; color: theme.colors.text }

                Flow { Layout.fillWidth: true; Layout.fillHeight: true; spacing: 8
                    CheatBtn { label: "\u6700\u5927\u91D1\u94B1"; action: "max_gold"; icon: "\uD83D\uDCB0"; color: theme.colors.warning }
                    CheatBtn { label: "\u4E00\u51FB\u5FC5\u6740"; action: "force_win"; icon: "\u2694"; color: theme.colors.danger }
                    CheatBtn { label: "\u5168\u5458\u6062\u590D"; action: "recover_all"; icon: "\uD83D\uDC9A"; color: theme.colors.success }
                    CheatBtn { label: "\u5168\u9053\u5177"; action: "all_items"; icon: "\uD83C\uDF92"; color: theme.colors.success }
                    CheatBtn { label: "\u795E\u6A21\u5F0F"; action: "god_mode"; icon: "\u26A1"; color: theme.colors.accent; isToggle: true }
                    CheatBtn { label: "\u7A7F\u5899"; action: "walk_walls"; icon: "\uD83D\uDC7B"; color: theme.colors.accent; isToggle: true }
                    CheatBtn { label: "\u53475\u7EA7"; action: "level_up"; icon: "\u2B50"; color: theme.colors.warning }
                    CheatBtn { label: "\u9003\u8DD1"; action: "force_escape"; icon: "\uD83C\uDFC3"; color: theme.colors.accent }
                }
            }
        }
    }

    component CheatBtn: Rectangle {
        id: cheat
        property string label: ""
        property string action: ""
        property string icon: ""
        property color color: theme.colors.accent
        property bool isToggle: false
        property bool active: false

        width: 100; height: 64; radius: theme.metrics.radiusSmall
        color: active ? color : theme.colors.surfaceAlt
        border.color: active ? color : theme.colors.border
        border.width: theme.metrics.borderWidth

        ColumnLayout { anchors.centerIn: parent; spacing: 2
            Text { text: icon; font.pixelSize: 18; Layout.alignment: Qt.AlignHCenter }
            Text { text: (active && isToggle ? "\u2713 " : "") + label; color: active ? "#fff" : (mainWindow.isConnected ? theme.colors.text : theme.colors.textMuted); font.pixelSize: 9; Layout.alignment: Qt.AlignHCenter }
        }

        MouseArea {
            anchors.fill: parent; cursorShape: Qt.PointingHandCursor; enabled: mainWindow.isConnected
            onClicked: {
                mainWindow.cheatAction(action)
                if (isToggle) active = !active
                else { active = true; flashTimer.start() }
            }
        }
        Timer { id: flashTimer; interval: 1200; onTriggered: active = false }
    }
}
```

- [ ] **Step 2: Commit**

```bash
git add qml/HomeTab.qml
git commit -m "feat: add HomeTab with game selection + cheat buttons"
```

---

### Task 4: Create TranslationTab.qml — 3 translation cards

**Files:**
- Create: `qml/TranslationTab.qml`

- [ ] **Step 1: Write TranslationTab.qml**

```qml
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
                    Rectangle { Layout.fillWidth: true; height: 32; radius: 6; color: theme.colors.inputBg; border.color: theme.colors.border
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
```

- [ ] **Step 2: Commit**

```bash
git add qml/TranslationTab.qml
git commit -m "feat: add TranslationTab with 3 translation cards"
```

---

### Task 5: Create MapTab.qml — current map info + teleport list

**Files:**
- Create: `qml/MapTab.qml`

- [ ] **Step 1: Write MapTab.qml**

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 14

        // Current Map
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 80; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.centerIn: parent; spacing: 4
                RowLayout { Layout.alignment: Qt.AlignHCenter; spacing: 8
                    Text { text: "\uD83D\uDCCD"; font.pixelSize: 20 }
                    Text { text: mainWindow.isConnected ? (mainWindow.currentMapInfo ? mainWindow.currentMapInfo.name || "\u5F53\u524D\u5730\u56FE" : "\u5F53\u524D\u5730\u56FE") : "\u672A\u8FDE\u63A5"; font.pixelSize: 14; font.bold: true; color: theme.colors.text }
                }
                Text { text: mainWindow.isConnected ? ("ID: " + ((mainWindow.currentMapInfo && mainWindow.currentMapInfo.id) ? mainWindow.currentMapInfo.id : "-")) : "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B"; color: theme.colors.textMuted; font.pixelSize: 11 }
            }
        }

        // Map List
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; radius: theme.metrics.radius
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 8
                Text { text: "\uD83D\uDDFA \u5730\u56FE\u4F20\u9001"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                ListView {
                    id: mapList
                    Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: mainWindow.mapListModel

                    delegate: Rectangle {
                        width: mapList.width; height: 36; radius: 6
                        color: index % 2 === 0 ? theme.colors.surfaceAlt : "transparent"

                        RowLayout { anchors.fill: parent; anchors.margins: 8; spacing: 10
                            Text { text: modelData; color: theme.colors.text; font.pixelSize: 12; Layout.fillWidth: true; elide: Text.ElideRight }
                            CButton { label: "\u4F20\u9001"; onClicked: mainWindow.teleportToMap(index + 1) }
                        }
                    }

                    Text { anchors.centerIn: parent; text: mainWindow.isConnected ? "\u65E0\u5730\u56FE\u6570\u636E" : "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B\u5730\u56FE\u5217\u8868"; color: theme.colors.textMuted; font.pixelSize: 12; visible: mapList.count === 0 }
                }
            }
        }
    }
}
```

- [ ] **Step 2: Commit**

```bash
git add qml/MapTab.qml
git commit -m "feat: add MapTab with current map info + teleport list"
```

---

### Task 6: Create VariableTab.qml — switches + variables grouped

**Files:**
- Create: `qml/VariableTab.qml`

- [ ] **Step 1: Write VariableTab.qml**

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 14

        // Switches
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredHeight: parent.height * 0.45
            radius: theme.metrics.radius; color: theme.colors.surface
            border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 6
                Text { text: "\uD83D\uDD18 \u5F00\u5173 (Switches)"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                RowLayout { Layout.fillWidth: true; spacing: 8
                    Text { text: "\u8DF3\u8F6C ID:"; color: theme.colors.textMuted; font.pixelSize: 11 }
                    Rectangle { width: 60; height: 24; radius: 4; color: theme.colors.inputBg; border.color: theme.colors.border
                        TextInput { id: switchJumpId; anchors.fill: parent; anchors.margins: 4; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 11; text: "1" }
                    }
                    CButton { label: "\u8DF3\u8F6C"; onClicked: { var idx = parseInt(switchJumpId.text) - 1; if (idx >= 0 && idx < switchList.count) switchList.positionViewAtIndex(idx, ListView.Beginning) } }
                }

                ListView {
                    id: switchList; Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: mainWindow.isConnected ? mainWindow.switchListModel : []
                    delegate: RowLayout {
                        width: switchList.width; height: 30; spacing: 8
                        Text { text: "#" + modelData.id; color: theme.colors.textMuted; font.pixelSize: 10; Layout.preferredWidth: 40 }
                        Rectangle { width: 10; height: 10; radius: 5; color: modelData.value ? theme.colors.success : theme.colors.textMuted }
                        Text { text: modelData.value ? "ON" : "OFF"; color: modelData.value ? theme.colors.success : theme.colors.textMuted; font.pixelSize: 10; Layout.preferredWidth: 30 }
                        CButton { label: "\u5F00"; isSuccess: true; onClicked: mainWindow.toggleSwitch(modelData.id, true) }
                        CButton { label: "\u5173"; isDanger: true; onClicked: mainWindow.toggleSwitch(modelData.id, false) }
                    }
                    Text { anchors.centerIn: parent; text: mainWindow.isConnected ? "" : "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B"; color: theme.colors.textMuted; font.pixelSize: 12; visible: !mainWindow.isConnected }
                }
            }
        }

        // Variables
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; Layout.preferredHeight: parent.height * 0.45
            radius: theme.metrics.radius; color: theme.colors.surface
            border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 6
                Text { text: "\uD83D\uDD22 \u53D8\u91CF (Variables)"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                RowLayout { Layout.fillWidth: true; spacing: 8
                    Text { text: "\u8DF3\u8F6C ID:"; color: theme.colors.textMuted; font.pixelSize: 11 }
                    Rectangle { width: 60; height: 24; radius: 4; color: theme.colors.inputBg; border.color: theme.colors.border
                        TextInput { id: varJumpId; anchors.fill: parent; anchors.margins: 4; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 11; text: "1" }
                    }
                    CButton { label: "\u8DF3\u8F6C"; onClicked: { var idx = parseInt(varJumpId.text) - 1; if (idx >= 0 && idx < varList.count) varList.positionViewAtIndex(idx, ListView.Beginning) } }
                }

                ListView {
                    id: varList; Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: mainWindow.isConnected ? mainWindow.variableListModel : []
                    delegate: RowLayout {
                        width: varList.width; height: 32; spacing: 8
                        Text { text: "#" + modelData.id; color: theme.colors.textMuted; font.pixelSize: 10; Layout.preferredWidth: 40 }
                        Rectangle { Layout.fillWidth: true; height: 26; radius: 4; color: theme.colors.inputBg; border.color: theme.colors.border
                            TextInput {
                                id: varEdit; anchors.fill: parent; anchors.leftMargin: 8
                                verticalAlignment: Text.AlignVCenter; color: theme.colors.text; font.pixelSize: 12
                                text: modelData.value
                            }
                        }
                        CButton { label: "\u4FEE\u6539"; onClicked: mainWindow.setVariable(modelData.id, parseInt(varEdit.text) || 0) }
                    }
                    Text { anchors.centerIn: parent; text: mainWindow.isConnected ? "" : "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B"; color: theme.colors.textMuted; font.pixelSize: 12; visible: !mainWindow.isConnected }
                }
            }
        }
    }
}
```

- [ ] **Step 2: Commit**

```bash
git add qml/VariableTab.qml
git commit -m "feat: add VariableTab with switches + variables grouped"
```

---

### Task 7: Create EventTab.qml — event browser (left list + right detail)

**Files:**
- Create: `qml/EventTab.qml`

- [ ] **Step 1: Write EventTab.qml**

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    RowLayout { anchors.fill: parent; spacing: 0

        // Left: Event List
        Rectangle {
            Layout.preferredWidth: parent.width * 0.35; Layout.fillHeight: true
            color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 12; spacing: 8
                Text { text: "\uD83D\uDCCB \u4E8B\u4EF6\u5217\u8868"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                // Common Events
                Text { text: "\u516C\u5171\u4E8B\u4EF6"; font.pixelSize: 11; font.bold: true; color: theme.colors.textMuted }
                ListView {
                    id: commonList; Layout.fillWidth: true; Layout.preferredHeight: parent.height * 0.4; clip: true
                    model: mainWindow.isConnected ? mainWindow.commonEventListModel : []
                    delegate: Rectangle {
                        width: commonList.width; height: 30; radius: 4
                        color: commonList.currentIndex === index ? theme.colors.surfaceAlt : "transparent"
                        Text { anchors.fill: parent; anchors.margins: 8; text: modelData; color: theme.colors.text; font.pixelSize: 11; verticalAlignment: Text.AlignVCenter }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { commonList.currentIndex = index; mainWindow.selectCommonEvent(index + 1) } }
                    }
                }

                // Map Events
                Text { text: "\u5F53\u524D\u5730\u56FE\u4E8B\u4EF6"; font.pixelSize: 11; font.bold: true; color: theme.colors.textMuted }
                ListView {
                    id: mapEventList; Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                    model: mainWindow.isConnected ? mainWindow.mapEventListModel : []
                    delegate: Rectangle {
                        width: mapEventList.width; height: 30; radius: 4
                        color: mapEventList.currentIndex === index ? theme.colors.surfaceAlt : "transparent"
                        Text { anchors.fill: parent; anchors.margins: 8; text: modelData; color: theme.colors.text; font.pixelSize: 11; verticalAlignment: Text.AlignVCenter }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { mapEventList.currentIndex = index; mainWindow.selectMapEvent(index + 1) } }
                    }
                }
            }
        }

        // Right: Event Detail
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: theme.colors.bg; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

            ColumnLayout { anchors.fill: parent; anchors.margins: 16; spacing: 10
                Text { text: "\uD83D\uDD0D \u4E8B\u4EF6\u8BE6\u60C5"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }

                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true; radius: theme.metrics.radius
                    color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                    ColumnLayout { anchors.fill: parent; anchors.margins: 14; spacing: 8
                        Text { text: mainWindow.selectedEventName || "\u9009\u62E9\u5DE6\u4FA7\u4E8B\u4EF6\u67E5\u770B\u8BE6\u60C5"; color: theme.colors.text; font.pixelSize: 13; font.bold: true }
                        Rectangle { width: parent.width; height: 1; color: theme.colors.border }

                        Text { text: mainWindow.selectedEventDetail || "\u6CA1\u6709\u9009\u4E2D\u4E8B\u4EF6"; color: theme.colors.textMuted; font.pixelSize: 11; Layout.fillWidth: true; wrapMode: Text.Wrap }

                        CButton {
                            label: "\u6267\u884C\u6B64\u4E8B\u4EF6"
                            isPrimary: true; enabled: mainWindow.isConnected && mainWindow.selectedEventId > 0
                            onClicked: mainWindow.triggerSelectedEvent()
                        }
                        Item { Layout.fillHeight: true }
                    }
                }
            }
        }
    }
}
```

- [ ] **Step 2: Commit**

```bash
git add qml/EventTab.qml
git commit -m "feat: add EventTab with event browser left list + right detail"
```

---

### Task 8: Create StatusTab.qml — game state dashboard

**Files:**
- Create: `qml/StatusTab.qml`

- [ ] **Step 1: Write StatusTab.qml**

```qml
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    ColumnLayout { anchors.fill: parent; anchors.margins: 20; spacing: 14

        Text { text: "\uD83D\uDCCA \u6E38\u620F\u72B6\u6001"; font.pixelSize: 16; font.bold: true; color: theme.colors.text }

        Item { Layout.fillWidth: true; Layout.fillHeight: true; visible: !mainWindow.isConnected
            ColumnLayout { anchors.centerIn: parent; spacing: 14
                Text { Layout.alignment: Qt.AlignHCenter; text: "\u2B21"; font.pixelSize: 52; color: theme.colors.textMuted; opacity: 0.3 }
                Text { Layout.alignment: Qt.AlignHCenter; text: "\u542F\u52A8\u6E38\u620F\u540E\u67E5\u770B\u72B6\u6001"; color: theme.colors.textMuted; font.pixelSize: 13 }
            }
        }

        ColumnLayout { Layout.fillWidth: true; spacing: 12; visible: mainWindow.isConnected

            // Party
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 140; radius: theme.metrics.radius
                color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                ColumnLayout { anchors.fill: parent; anchors.margins: 14; spacing: 6
                    Text { text: "\uD83D\uDC65 \u961F\u4F0D"; font.pixelSize: 13; font.bold: true; color: theme.colors.text }
                    Rectangle { width: parent.width; height: 1; color: theme.colors.border }
                    Flow { Layout.fillWidth: true; spacing: 8
                        Repeater {
                            model: mainWindow.partyStatusModel
                            delegate: Rectangle {
                                width: 160; height: 50; radius: 8
                                color: theme.colors.surfaceAlt; border.color: theme.colors.border; border.width: theme.metrics.borderWidth
                                ColumnLayout { anchors.fill: parent; anchors.margins: 8; spacing: 2
                                    Text { text: modelData.name || ("\u89D2\u8272" + (index + 1)); color: theme.colors.text; font.pixelSize: 11; font.bold: true }
                                    Text { text: "Lv." + (modelData.level || "?") + "  HP " + (modelData.hp || "?") + "/" + (modelData.maxHp || "?"); color: theme.colors.textMuted; font.pixelSize: 10 }
                                }
                            }
                        }
                    }
                }
            }

            // Map Info
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 70; radius: theme.metrics.radius
                color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                RowLayout { anchors.fill: parent; anchors.margins: 14; spacing: 12
                    Text { text: "\uD83D\uDDFA"; font.pixelSize: 24 }
                    ColumnLayout {
                        Text { text: mainWindow.currentMapName || "\u5F53\u524D\u5730\u56FE"; color: theme.colors.text; font.pixelSize: 13; font.bold: true }
                        Text { text: "ID: " + (mainWindow.currentMapId || "-") + "  \u5750\u6807: (" + (mainWindow.playerX || "-") + ", " + (mainWindow.playerY || "-") + ")"; color: theme.colors.textMuted; font.pixelSize: 10 }
                    }
                }
            }

            // Connection
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 60; radius: theme.metrics.radius
                color: theme.colors.surface; border.color: theme.colors.border; border.width: theme.metrics.borderWidth

                RowLayout { anchors.fill: parent; anchors.margins: 14; spacing: 12
                    Rectangle { width: 10; height: 10; radius: 5; color: theme.colors.success }
                    Text { text: "\u5DF2\u8FDE\u63A5"; color: theme.colors.success; font.pixelSize: 13; font.bold: true }
                    Item { Layout.fillWidth: true }
                    Text { text: mainWindow.translationStats || ""; color: theme.colors.textMuted; font.pixelSize: 11 }
                }
            }

            Item { Layout.fillHeight: true }
        }
    }
}
```

- [ ] **Step 2: Commit**

```bash
git add qml/StatusTab.qml
git commit -m "feat: add StatusTab with party + map + connection info"
```

---

### Task 9: Add Q_INVOKABLE stubs to MainWindow for tab data

**Files:**
- Modify: `src/app/MainWindow.h`
- Modify: `src/app/MainWindow.cpp`

- [ ] **Step 1: Add new properties and methods to MainWindow.h**

Replace the MainWindow.h Q_PROPERTY and Q_INVOKABLE sections:

Add after `Q_PROPERTY(QString translationStats READ translationStats NOTIFY translationStatsChanged)`:

```cpp
    Q_PROPERTY(QString currentMapName READ currentMapName NOTIFY currentMapNameChanged)
    Q_PROPERTY(QString currentMapId READ currentMapId NOTIFY currentMapIdChanged)
    Q_PROPERTY(QString playerX READ playerX NOTIFY playerXChanged)
    Q_PROPERTY(QString playerY READ playerY NOTIFY playerYChanged)
    Q_PROPERTY(QString selectedEventName READ selectedEventName NOTIFY selectedEventNameChanged)
    Q_PROPERTY(QString selectedEventDetail READ selectedEventDetail NOTIFY selectedEventDetailChanged)
    Q_PROPERTY(int selectedEventId READ selectedEventId NOTIFY selectedEventIdChanged)
    Q_PROPERTY(QVariantList mapListModel READ mapListModel NOTIFY mapListModelChanged)
    Q_PROPERTY(QVariantList switchListModel READ switchListModel NOTIFY switchListModelChanged)
    Q_PROPERTY(QVariantList variableListModel READ variableListModel NOTIFY variableListModelChanged)
    Q_PROPERTY(QVariantList commonEventListModel READ commonEventListModel NOTIFY commonEventListModelChanged)
    Q_PROPERTY(QVariantList mapEventListModel READ mapEventListModel NOTIFY mapEventListModelChanged)
    Q_PROPERTY(QVariantList partyStatusModel READ partyStatusModel NOTIFY partyStatusModelChanged)
    Q_PROPERTY(QVariantMap currentMapInfo READ currentMapInfo NOTIFY currentMapInfoChanged)
```

Add after existing Q_INVOKABLE methods:

```cpp
    // Map tab
    Q_INVOKABLE void teleportToMap(int mapId);
    // Variable tab
    Q_INVOKABLE void toggleSwitch(int id, bool on);
    Q_INVOKABLE void setVariable(int id, int value);
    // Event tab
    Q_INVOKABLE void selectCommonEvent(int id);
    Q_INVOKABLE void selectMapEvent(int id);
    Q_INVOKABLE void triggerSelectedEvent();
```

Add after existing signals:

```cpp
    // Tab data signals
    void currentMapNameChanged(); void currentMapIdChanged();
    void playerXChanged(); void playerYChanged();
    void selectedEventNameChanged(); void selectedEventDetailChanged();
    void selectedEventIdChanged();
    void mapListModelChanged(); void switchListModelChanged();
    void variableListModelChanged(); void commonEventListModelChanged();
    void mapEventListModelChanged(); void partyStatusModelChanged();
    void currentMapInfoChanged();
```

Add to private section:

```cpp
    // Tab data state
    QString m_currentMapName; QString m_currentMapId;
    QString m_playerX; QString m_playerY;
    QString m_selectedEventName; QString m_selectedEventDetail;
    int m_selectedEventId;
    QVariantList m_mapListModel; QVariantList m_switchListModel;
    QVariantList m_variableListModel; QVariantList m_commonEventListModel;
    QVariantList m_mapEventListModel; QVariantList m_partyStatusModel;
    QVariantMap m_currentMapInfo;
```

Add getter/accessor signatures in public section:

```cpp
    QString currentMapName() const; QString currentMapId() const;
    QString playerX() const; QString playerY() const;
    QString selectedEventName() const; QString selectedEventDetail() const;
    int selectedEventId() const;
    QVariantList mapListModel() const; QVariantList switchListModel() const;
    QVariantList variableListModel() const; QVariantList commonEventListModel() const;
    QVariantList mapEventListModel() const; QVariantList partyStatusModel() const;
    QVariantMap currentMapInfo() const;
```

- [ ] **Step 2: Implement stubs in MainWindow.cpp**

Add after existing getters (around line 59):

```cpp
// Tab data stubs — return empty/default data initially
QString MainWindow::currentMapName() const { return m_currentMapName; }
QString MainWindow::currentMapId() const { return m_currentMapId; }
QString MainWindow::playerX() const { return m_playerX; }
QString MainWindow::playerY() const { return m_playerY; }
QString MainWindow::selectedEventName() const { return m_selectedEventName; }
QString MainWindow::selectedEventDetail() const { return m_selectedEventDetail; }
int MainWindow::selectedEventId() const { return m_selectedEventId; }
QVariantList MainWindow::mapListModel() const { return m_mapListModel; }
QVariantList MainWindow::switchListModel() const { return m_switchListModel; }
QVariantList MainWindow::variableListModel() const { return m_variableListModel; }
QVariantList MainWindow::commonEventListModel() const { return m_commonEventListModel; }
QVariantList MainWindow::mapEventListModel() const { return m_mapEventListModel; }
QVariantList MainWindow::partyStatusModel() const { return m_partyStatusModel; }
QVariantMap MainWindow::currentMapInfo() const { return m_currentMapInfo; }
```

Add after existing Q_INVOKABLE implementations (after line 168):

```cpp
// ---- Tab data stubs ----
void MainWindow::teleportToMap(int mapId) {
    // Stub: send write command for map teleport
    m_cheatController->writeState("player.mapId", mapId);
}

void MainWindow::toggleSwitch(int id, bool on) {
    QString path = QString("switches.%1").arg(id);
    writeGameState(path, QVariant(on));
}

void MainWindow::setVariable(int id, int value) {
    QString path = QString("variables.%1").arg(id);
    writeGameState(path, QVariant(value));
}

void MainWindow::selectCommonEvent(int id) {
    m_selectedEventId = id;
    m_selectedEventName = QString("公共事件 #%1").arg(id);
    m_selectedEventDetail = QString("事件 ID: %1\n类型: 公共事件\n\n功能开发中...").arg(id);
    emit selectedEventNameChanged(); emit selectedEventDetailChanged();
    emit selectedEventIdChanged();
}

void MainWindow::selectMapEvent(int id) {
    m_selectedEventId = id;
    m_selectedEventName = QString("地图事件 #%1").arg(id);
    m_selectedEventDetail = QString("事件 ID: %1\n类型: 地图事件\n\n功能开发中...").arg(id);
    emit selectedEventNameChanged(); emit selectedEventDetailChanged();
    emit selectedEventIdChanged();
}

void MainWindow::triggerSelectedEvent() {
    m_cheatController->sendCheat(QString("trigger_event_%1").arg(m_selectedEventId));
}
```

- [ ] **Step 3: Commit**

```bash
git add src/app/MainWindow.h src/app/MainWindow.cpp
git commit -m "feat: add Q_INVOKABLE stubs for tab data accessors"
```

---

### Task 10: Build and verify

**Files:** None (build verification)

- [ ] **Step 1: Build the project**

```powershell
$env:PATH = "C:\Qt\5.15.2\mingw81_64\bin;E:\mingw\mingw64\bin;$env:PATH"
cmake --build build --target all 2>&1
```

Expected: Build succeeds with 0 errors, 0 warnings.

- [ ] **Step 2: Run tests**

```powershell
cmake --build build --target test 2>&1
```

Expected: All 47 tests pass.

- [ ] **Step 3: Run windeployqt**

```powershell
windeployqt --qmldir E:\Rtranslator_rebuild\qml E:\Rtranslator_rebuild\build\RTranslator.exe --no-translations
```

- [ ] **Step 4: Commit if needed**

```bash
git add -A
git diff --cached --stat
git commit -m "chore: build verification after UI redesign"
```
