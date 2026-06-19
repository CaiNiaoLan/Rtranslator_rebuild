# RTranslator UI Redesign — Design Specification

**Date:** 2026-06-19
**Status:** Approved
**Target:** Replace 3-column layout with tab-based navigation + functional layout

---

## 1. Overview

Redesign RTranslator's UI from a rigid 3-column layout to a flexible tab-based navigation system. Each tab is a dedicated workspace for a specific function group. The multi-theme system (5 themes) is preserved.

### 1.1 Key Changes from Current

| Current | New |
|---------|-----|
| 3-column (Translation | Modification | Game State) | 6-tab top bar with StackLayout |
| QML embedded in main.qml (270 lines) | main.qml is layout shell; tab content in separate QML files |
| Cheats + modification mixed in center panel | Cheats on Home tab; detailed modification in Variable tab |
| Game state static placeholders | Status tab with live data |
| No map/event functionality in UI | Map tab + Event tab (new) |

---

## 2. Navigation

### 2.1 Top Tab Bar

6 tabs, displayed as a horizontal row below the top toolbar:

```
┌──────────────────────────────────────────────────────────┐
│ RTranslator           [路径...]  [浏览] [启动] [断开] [🎨]│ ← top bar
├──────────────────────────────────────────────────────────┤
│ 🏠 主页 │ 🌐 翻译 │ 🗺 地图 │ 🔀 变量 │ 🎭 事件 │ 📊 状态│ ← tab bar
├──────────────────────────────────────────────────────────┤
│                                                          │
│                    Tab content area                      │
│                                                          │
├──────────────────────────────────────────────────────────┤
│ ● 已连接                     R:\Game\Game.exe            │ ← bottom bar
└──────────────────────────────────────────────────────────┘
```

- Tabs use `TabBar` + `StackLayout` from QtQuick.Controls 2
- Active tab has accent color highlight
- Tab switching preserves state (no re-creation on switch)
- Window minimum size: 1040×680

### 2.2 Top Toolbar (unchanged, clean up)

- App name "RTranslator" on left
- Game path display (read-only, drag-drop target)
- Buttons: 浏览 (Browse), 启动 (Launch), 断开 (Disconnect)
- Theme cycle button 🎨

### 2.3 Bottom Status Bar (unchanged)

- Connection dot (green pulse when connected)
- Connection status text
- Current game path (elided)

---

## 3. Tab Specifications

### 3.1 🏠 Home Tab

**Purpose:** Game selection + one-click cheats. First view on launch.

**Layout:** Vertical (top → bottom)

```
┌──────────────────────────────────────────┐
│ 🎮 选择游戏                               │
│ ┌──────────────────────────────────────┐ │
│ │ 游戏路径...                  [浏览]   │ │
│ │ 或拖拽 exe 到窗口                    │ │
│ │              [▶ 启动游戏]            │ │
│ └──────────────────────────────────────┘ │
│                                          │
│ ⚡ 一键作弊                               │
│ ┌────────┐ ┌────────┐ ┌────────┐        │
│ │💰 最大  │ │⚔ 一击 │ │💚 全员 │        │
│ │  金钱   │ │  必杀  │ │  恢复  │  ...   │
│ └────────┘ └────────┘ └────────┘        │
└──────────────────────────────────────────┘
```

**Components:**
- Game selection card: icon + path display + Browse/Launch buttons + drag-drop hint
- Cheat buttons flow: 6-8 buttons arranged horizontally in Flow layout
- Cheat actions: `recover_all`, `max_gold`, `all_items`, `god_mode`, `force_win`, `force_escape`, `walk_walls`, `level_up`

**Behavior:**
- Browse opens file dialog for `.exe`, sets `mainWindow.gamePath`
- Launch starts the game process
- Cheat buttons call `mainWindow.cheatAction(name)` — show brief success flash
- Toggle cheats (god_mode, walk_walls) show active/inactive state

---

### 3.2 🌐 Translation Tab

**Purpose:** Translation file management (load, extract, export).

**Layout:** 3 independent cards (vertical stack)

**Card 1 — 加载翻译文件 (Load Translation)**
- Translation JSON path input + Browse button
- Load button → calls `mainWindow.loadTranslationFile(path)`
- Shows loaded count on success (`"✓ 1234条"`)

**Card 2 — 提取原文 (Extract Original)**
- Extracts all translatable strings from game data files
- Shows progress or extracted count
- Extract button → calls `mainWindow.exportOriginalText()`

**Card 3 — 导出原文 (Export)**
- Shows translation statistics (coverage %)
- Export button to save extracted strings as JSON template

**Behavior:**
- Loaded translation file path and stats persist across tab switches
- Cards are always visible (no collapse needed — 3 cards fit in one view)

---

### 3.3 🗺 Map Tab

**Purpose:** View current map + teleport to any map.

**Layout:** Top information + bottom list

```
┌──────────────────────────────────────────┐
│ 📍 当前地图                               │
│ ┌──────────────────────────────────────┐ │
│ │ 地图名称: Forest Village  ID: 002    │ │
│ │ 地图尺寸: 45×30                      │ │
│ └──────────────────────────────────────┘ │
│                                          │
│ 🗺 地图传送                               │
│ ┌──────────────────────────────────────┐ │
│ │ Map001  草原                         │ │
│ │ Map002  森林村  ← 当前                │ │
│ │ Map003  洞窟                         │ │
│ │ Map004  王城                          │ │
│ │ ...                                  │ │
│ └──────────────────────────────────────┘ │
└──────────────────────────────────────────┘
```

**Components:**
- Current map info card (name, ID, size) — auto-updated via WebSocket
- Map list (ScrollView with ListView), each row: ID + name
- Selected map has "传送" button → calls write path to teleport player
- Current map row is highlighted

**Behavior:**
- Map list populated from `$dataMapInfos` via game state read
- Clicking a map row selects it; "传送" button changes player position
- Current map refreshes periodically when connected

---

### 3.4 🔀 Variable Tab

**Purpose:** View and modify switches + variables. Cheat attributes (gold/HP/MP) stay on Home tab.

**Layout:** Grouped sections, each collapsible

```
┌──────────────────────────────────────────┐
│ 🔘 开关 (Switches)                   [展开]│
│ ┌──────────────────────────────────────┐ │
│ │ #001 ⬜ OFF  [#001 ▸ ON ▸ OFF]       │ │
│ │ #002 🟢 ON   [#002 ▸ ON ▸ OFF]       │ │
│ │ #003 ⬜ OFF  [#003 ▸ ON ▸ OFF]       │ │
│ │ ... [显示前50个]                      │ │
│ └──────────────────────────────────────┘ │
│                                          │
│ 🔢 变量 (Variables)                  [展开]│
│ ┌──────────────────────────────────────┐ │
│ │ #001  42    修改: [_42_] [修改]       │ │
│ │ #002  0     修改: [_0__] [修改]       │ │
│ │ #003  156   修改: [_156] [修改]       │ │
│ │ ... [显示前50个]                      │ │
│ └──────────────────────────────────────┘ │
└──────────────────────────────────────────┘
```

**Components:**
- Switches section: ListView with ID + ON/OFF toggle button, show first 50
- Variables section: ListView with ID + value + inline edit field + "修改" button, show first 50
- Both sections are collapsible (tap header to expand/collapse)
- Quick jump: input field to jump to a specific switch/variable ID

**Behavior:**
- Pressing ON/OFF sends `writeGameState("switches.N", true/false)`
- Variable modify sends `writeGameState("variables.N", value)`
- Sections start collapsed; tap to expand
- Connected state required; show disabled overlay when offline

---

### 3.5 🎭 Event Tab

**Purpose:** Browse common events + current map events, view event flow, trigger events.

**Layout:** Left list + right detail (split view)

```
┌────────────────────┬──────────────────────┐
│ 📋 事件列表          │ 🔍 事件详情           │
│                    │                      │
│ 公共事件            │ 事件名称: 开场剧情      │
│ ┌────────────────┐ │ ID: 1  ·  类型: 公共  │
│ │ ⊳ 开场剧情      │ │                      │
│ │ ⊳ BOSS战       │ │ 事件页: [1] [2]       │
│ │ ⊳ 商店对话      │ │                      │
│ └────────────────┘ │ 页面条件:              │
│                    │  独立开关 A = ON       │
│ 当前地图事件         │                      │
│ ┌────────────────┐ │ 指令流程:              │
│ │ ⊳ 村民对话      │ │ ▶ 显示文字:"你好"     │
│ │ ⊳ 宝箱         │ │ ▶ 选项:"是/否"        │
│ │ ⊳ 传送阵       │ │   → 是: 获得道具       │
│ │ ⊳ 守卫         │ │   → 否: 结束           │
│ └────────────────┘ │ ▶ ...                │
│                    │                      │
│                    │ [执行此事件]           │
└────────────────────┴──────────────────────┘
```

**Components:**
- Left pane (∼35% width): two lists
  - "公共事件" header + list items
  - "当前地图事件" header + list items
  - Each item shows event name; current active page dot indicator
- Right pane (∼65% width): detail view
  - Event name, ID, type
  - Event pages as tabs/pills (click to switch page view)
  - Page conditions display (self-switch A/B/C/D, switch requirements, variable check)
  - Instruction list (command-by-command tree view for ShowText, Choices, etc.)
  - "执行此事件" button — force-triggers the selected event page

**Behavior:**
- Click event in left list → loads detail in right pane
- Event page pills show active/inactive state
- Instruction tree renders branch structure visually (indentation for nested conditions)
- Force trigger calls `writeGameState` to start the event
- Falls back gracefully if not connected or event data unavailable

---

### 3.6 📊 Status Tab

**Purpose:** Real-time game state dashboard.

**Layout:** Stack of info cards

```
┌──────────────────────────────────────────┐
│ 👥 队伍                                   │
│ ┌──────────────────────────────────────┐ │
│ │ 角色1: 勇者   Lv.5   HP 120/150      │ │
│ │ 角色2: 法师   Lv.4   HP 80/80        │ │
│ │ 角色3: 僧侣   Lv.4   HP 65/70        │ │
│ │ 金币: 9999G                          │ │
│ └──────────────────────────────────────┘ │
│                                          │
│ 🗺 地图                                   │
│ ┌──────────────────────────────────────┐ │
│ │ 当前: Map002 森林村                   │ │
│ │ 坐标: (12, 8)                        │ │
│ └──────────────────────────────────────┘ │
│                                          │
│ 🔌 连接                                   │
│ ┌──────────────────────────────────────┐ │
│ │ WebSocket: 已连接                     │ │
│ │ 翻译: 1234 条 / 45%                   │ │
│ └──────────────────────────────────────┘ │
└──────────────────────────────────────────┘
```

**Components:**
- Party card: member list with name, level, HP/MP, gold
- Map card: current map, coordinates
- Connection card: WS status, translation stats
- When offline: single centered message "启动游戏后查看状态"

**Behavior:**
- Auto-refreshes when connected (poll interval ~2s for party, event-driven for map changes)
- Falls back to offline placeholder when disconnected

---

## 4. Theme System (Preserved)

5 themes from existing `ThemeManager` are kept:
- Aurora (暗紫电竞)
- Glassmorphism (毛玻璃)
- Brutalism (粗野)
- Bento (暖米日系)
- Minimalism (极简)

**Changes:**
- All theme tokens (`colors.*`, `metrics.*`) remain as context properties
- All new QML components use `theme.colors.X` and `theme.metrics.X`
- Theme cycle button 🎨 stays in top toolbar
- QSettings persistence unchanged

---

## 5. QML File Reorganization

### 5.1 New File Structure

```
qml/
├── main.qml              # Root Window: toolbar + TabBar + StackLayout + status bar
├── CButton.qml           # Reusable themed button (unchanged)
├── SectionCard.qml       # Reusable themed card (unchanged)
├── HomeTab.qml           # Home tab content (game select + cheats)
├── TranslationTab.qml    # Translation tab content (3 cards)
├── MapTab.qml            # Map tab content (current + list)
├── VariableTab.qml       # Variable tab content (switches + variables grouped)
├── EventTab.qml          # Event tab content (left list + right detail)
├── StatusTab.qml         # Status tab content (info cards)
└── qml.qrc               # Updated to include all files
```

### 5.2 Files to Remove / Archive

Following files will be replaced by tab equivalents:
- `TopBar.qml` → merged into `main.qml` toolbar
- `StatusBar.qml` → merged into `main.qml` bottom bar
- `TranslationPanel.qml` → `TranslationTab.qml`
- `ModificationPanel.qml` → split into `HomeTab.qml` (cheats) + `VariableTab.qml` (switches/vars)
- `GameStatePanel.qml` → `StatusTab.qml`
- `CheatBtn.qml` → replaced by `CButton` (already exists)

Note: These files are NOT in qrc currently but exist on disk. They should be deleted to avoid confusion.

### 5.3 qml.qrc

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

---

## 6. C++ / MainWindow Changes

### 6.1 New Q_INVOKABLE Methods

```cpp
// Map tab
Q_INVOKABLE QStringList mapList() const;           // Returns list of map names from adapter
Q_INVOKABLE QVariantMap currentMapInfo() const;    // {name, id, width, height}
Q_INVOKABLE void teleportToMap(int mapId, int x, int y);

// Event tab
Q_INVOKABLE QVariantList commonEventList() const;  // [{id, name}]
Q_INVOKABLE QVariantList mapEventList() const;     // [{id, name}] for current map
Q_INVOKABLE QVariantMap eventDetail(int eventId, bool isCommon) const; // event pages + commands
Q_INVOKABLE void triggerEvent(int eventId, int pageIndex);

// Variable tab
Q_INVOKABLE QVariantList switchList() const;       // [{id, value}] queried via WebSocket
Q_INVOKABLE QVariantList variableList() const;     // [{id, value}]
Q_INVOKABLE void toggleSwitch(int id, bool on);
Q_INVOKABLE void setVariable(int id, int value);
```

### 6.2 Preserved Methods (no change)

- `setGamePath()`, `gamePath()`, `browseGameFile()`
- `launchGame()`, `detachGame()`
- `isConnected()` property
- `loadTranslationFile()`, `exportOriginalText()`
- `cheatAction()`, `writeGameState()`, `triggerBattleVictory()`
- `translationStats` property

---

## 7. Drag-and-Drop (Preserved)

DropArea covers the full window, accepts `.exe` files, URI decoding unchanged. This feature moves from `main.qml` to the root Window level (already there).

---

## 8. Data Flow

```
User clicks tab → StackLayout switches to tab QML
Tab QML shows placeholders → binds to MainWindow properties
Game connects → WebSocket state reader sends live data
Tab QML updates via property bindings
User action → calls Q_INVOKABLE → writes to game via WebSocket → tab reflects change
```

No polling in tabs except Status tab (2s party refresh). All other data is event-driven or on-demand.

---

## 9. Implementation Notes

### 9.1 StackLayout vs Loader

Use `StackLayout` with `currentIndex` bound to `TabBar.currentIndex`. All tabs are created eagerly so switching is instant.

### 9.2 Component Reuse

- `CButton.qml` — used for all buttons (browse, launch, cheat, modify, etc.)
- `SectionCard.qml` — used for info/input card rows in Variable tab
- No new reusable components needed beyond these two

### 9.3 States

Each tab handles:
- **Disconnected** (no game): show placeholder / disabled state
- **Connected** (game running): show live data / enabled actions
- **Loading** (data fetching): show spinner or SkeletonText placeholder

### 9.4 Error Handling

- Tab data fetch failures: show error text in the tab area, not a modal
- WebSocket timeout: fall back to disconnected state
- Invalid input (e.g., non-numeric variable value): show inline validation, don't send

---

## 10. Scope Boundaries

**In scope:**
- All 6 tabs with layouts as specified
- Theme system adaptation for new components
- C++ bridge methods for new tab data
- QML file reorganization + qrc update

**Out of scope (future):**
- RPG Maker XP/VX/VX Ace adapter UI
- Machine translation API integration
- In-game text editor / inline translation editing
- Plugin text extraction configuration
- Multi-language switching UI

---

## 11. Testing

- Visual verification: each tab renders correctly across 5 themes
- Theme cycle: switching themes doesn't break tab layout
- Offline state: each tab shows appropriate placeholder when disconnected
- Tab switching: state preserved across tab switches (e.g., variable list doesn't reload)
- Drag-drop: still works from any tab
- Build: compiles with 0 warnings; existing 47 unit tests still pass
- windeployqt: includes all QML files for deployment
