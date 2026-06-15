# RTranslator — Design Specification

**Date:** 2026-06-15
**Status:** Draft
**Target:** RPG Maker MV & MZ (extensible to XP/VX/VX Ace)

---

## 1. Overview

RTranslator is a C++ desktop application for translating and modifying RPG Maker MV/MZ games at runtime. It is inspired by the game tooling software "mtool" and provides:

- **Translation overlay** — load a translation JSON file; all user-visible in-game text is replaced in real-time
- **Runtime modification** — read and modify game state (gold, stats, switches, variables) while playing
- **One-click battle victory** — instantly defeat all enemies in combat

The tool does **not** modify game files. Translation and modifications are injected at runtime via the Chrome DevTools Protocol (CDP) into the NW.js game process.

---

## 2. Engine Scope

| Version | Support |
|---------|---------|
| RPG Maker MV | Full |
| RPG Maker MZ | Full |
| RPG Maker VX Ace | Architecture reserved, future |
| RPG Maker XP | Architecture reserved, future |

MV and MZ both use NW.js (Chromium + Node.js) with JSON-format game data. The architecture is designed with abstractions (`IGameAdapter`, `IInjectionStrategy`) to allow plugging in different engine backends later.

---

## 3. Architecture

```
┌──────────────────────────┐         ┌────────────────────────────┐
│  RTranslator (Qt C++)    │  CDP    │  Game Process (NW.js)      │
│                          │────────>│                            │
│  ┌────────────────────┐  │ inject  │  ┌──────────────────────┐  │
│  │ QML UI             │  │         │  │ Injected Hook Script │  │
│  │ (Multi-panel)      │  │         │  │  • Text Interceptor  │  │
│  └────────────────────┘  │         │  │  • State Reader/Writer│  │
│  ┌────────────────────┐  │         │  └──────────────────────┘  │
│  │ Core Services      │  │         │  ┌──────────────────────┐  │
│  │ • GameManager      │  │  WS IPC │  │ WebSocket Client     │  │
│  │ • TranslationEngine│  │<───────>│  │ (bidi to Qt)         │  │
│  │ • CheatController  │  │         │  └──────────────────────┘  │
│  └────────────────────┘  │         │  ┌──────────────────────┐  │
│  ┌────────────────────┐  │         │  │ RPG Maker Core       │  │
│  │ IPC Bridge         │  │         │  │ $gameParty, etc.     │  │
│  │ • WebSocket Server │  │         │  └──────────────────────┘  │
│  │ • CDP Client       │  │         └────────────────────────────┘
│  └────────────────────┘  │
└──────────────────────────┘
```

### 3.1 Key Design Decisions

- **CDP Injection (not file patching):** No game files are modified. The Qt app launches the game with `--remote-debugging-port` and injects hook scripts via `Runtime.evaluate`.
- **WebSocket IPC:** Bidirectional communication between Qt (server) and injected JS (client). JSON protocol.
- **Qt 6 + QML:** Modern, responsive UI with native performance.
- **Dot-path state accessor:** Read/write any game state via paths like `party.gold`, `actors.1.hp`, `switches.42`.

### 3.2 Connection Lifecycle

1. Qt launches `Game.exe` with `--remote-debugging-port=9222`
2. Qt CDP client connects to `ws://127.0.0.1:9222/devtools/page/{id}`
3. Qt injects hook script via `Runtime.evaluate`
4. Injected JS opens WebSocket to `ws://127.0.0.1:{qt-port}`
5. Qt sends `init` message with translation map; hook begins intercepting text
6. Bidirectional `cmd`/`state` messages flow for read/write operations

---

## 4. Translation System

### 4.1 Two-Phase Model

**Phase 1 — Offline Extraction (Qt side):**
- Parse all game data JSON files (`Actors.json`, `Items.json`, `Skills.json`, `MapXXX.json`, `CommonEvents.json`, `System.json`, etc.)
- Extract strings only from whitelisted translatable fields (see field table below)
- Deduplicate strings, assign context metadata (`id`, `source`, `context`, `field`)
- Build an original-text hash set (the "whitelist" for runtime safety)

**Phase 2 — Runtime Interception (Injected JS side):**
- Hook the following RPG Maker methods:
  - `Window_Base.prototype.drawText` — item names, skill names, menu labels
  - `Window_Base.prototype.drawTextEx` — dialogue with escape codes
  - `Game_Message.prototype.add` — raw message text (before escape processing)
  - `Window_ChoiceList` start — choice/option text
- Each string to be displayed is checked against the Translation Map (hash table)
- **Found in map → replace with translation. NOT found → pass through unchanged.**

### 4.2 Translation Safety: Whitelist Rule

Only strings that were pre-extracted from game data files are eligible for translation. This guarantees we never accidentally translate:
- Escape codes (`\V[42]`, `\C[3]`, `\N[1]`)
- Numeric strings used as variable values
- Internal identifiers and keys
- Damage formulas (`a.atk * 4 - b.def * 2`)
- Plugin configuration parameters

### 4.3 Field Extraction Whitelist

| JSON File | Extracted Fields (Translatable) | NOT Extracted (Logic/Internal) |
|-----------|--------------------------------|-------------------------------|
| **Actors** | name, nickname, profile, note | id, classId, initialLevel, traits[] |
| **Classes** | name, description, note | expParams[], traits[], learnings[] |
| **Skills/Items/Weapons/Armors** | name, description, note, message1, message2 | damage.formula, effects[], traits[] |
| **Enemies** | name, note | params[], actions[], dropItems[] |
| **States** | name, message1-4, note | traits[], restriction, autoRemovalTiming |
| **System** | gameTitle, currencyUnit, terms (all subfields), elements[] | windowTone, shipBgm, switches[], variables[] |
| **MapXXX** | events[].pages[].list[].parameters[] (text in ShowText, ShowChoices commands) | command codes, switch/variable ids |
| **CommonEvents** | list[].parameters[] (same pattern), name | trigger, switchId |

### 4.4 Translation JSON Format

```json
{
  "meta": { "game": "GameTitle", "lang": "ja→zh" },
  "strings": {
    "こんにちは": "你好",
    "冒険に出かけよう！": "出发冒险吧！",
    "\\C[2]火魔法\\C[0]": "\\C[2]Fire Magic\\C[0]"
  }
}
```

- Keys include escape codes intact (match full string, not stripped text)
- Multi-line messages are matched at `Game_Message.add` level (before rendering splits)
- Template patterns with `\V[n]` are matched as whole templates

### 4.5 Translation Workflow UX

1. **Load Game** — select game folder, auto-detect via `www/data/System.json`
2. **Extract Text** — tool parses all data JSON, extracts strings with context metadata
3. **View & Export** — sortable/searchable table grouped by source file; export to JSON template
4. **Translate** — user fills translations externally or in the built-in editor
5. **Load + Launch** — tool loads translation JSON, launches game with hooks

---

## 5. Modification System

### 5.1 Cheat Features

| Feature | Implementation |
|---------|---------------|
| **Numeric values** | Read/write gold, HP/MP/ATK/DEF, item counts, exp, level via path accessor |
| **Switches & Variables** | Toggle event switches, set variable values |
| **Battle victory** | Set all enemy HP to 0; let engine's natural settlement logic run |
| **Items & Equipment** | Add/remove items, weapons, armors to inventory |

### 5.2 Path-based State Accessor

```
party.gold        → $gameParty.gold()
actors.1.hp       → $gameActors.actor(1).hp
actors.1.param.2  → $gameActors.actor(1).param(2)
switches.42       → $gameSwitches.value(42)
variables.7       → $gameVariables.value(7)
items.5.count     → $gameParty.numItems($dataItems[5])
```

### 5.3 Battle One-Shot

```javascript
if ($gameParty.inBattle()) {
  $gameTroop.members().forEach(e => e.setHp(0));
}
// Engine's natural battle-end logic handles exp, drops, victory message
```

---

## 6. IPC Protocol

### 6.1 Command Messages (Qt → Game)

| Type | Payload | Effect |
|------|---------|--------|
| `init` | `{ map: {...}, hooks: [...] }` | Setup translation map and install hooks |
| `read` | `{ path: "party.gold" }` | Read game state value |
| `write` | `{ path: "party.gold", value: 99999 }` | Set game state value |
| `batch` | `{ ops: [{read...}, {write...}] }` | Multiple operations atomically |
| `battle_victory` | `{}` | Set all enemy HP to 0 |
| `add_item` | `{ id: 10, type: "item", count: 1 }` | Add item/weapon/armor |
| `reload_trans` | `{ map: {...} }` | Hot-reload translation map |

### 6.2 Response Messages (Game → Qt)

| Type | Payload | Trigger |
|------|---------|---------|
| `state` | `{ path: "...", value: ... }` | Response to read command |
| `ack` | `{ cmd: "write", success: true }` | Confirms write/add_item |
| `notify` | `{ event: "translated", text: "..." }` | Optional translation log |
| `error` | `{ cmd: "...", reason: "..." }` | Failed operation |

### 6.3 State Guarantees

- **Atomic batches:** batch of writes all succeed or all fail
- **Read-after-write:** every write returns the new value in the ack
- **Staleness detection:** read ops include sequence ID; state change = notify
- **No blind overwrites:** invalid paths return error, no corruption

---

## 7. UI Design

### 7.1 Layout: Multi-Panel (Option B)

```
┌─────────────────────────────────────────────────────────────┐
│ RTranslator    [Game.exe path...] [Launch] [Detach]         │  ← Top Bar
├──────────────────────┬──────────────────┬───────────────────┤
│ Translation          │ Modification     │ Game State        │
│ ┌──────────────────┐ │ ┌──────────────┐ │ ┌───────────────┐ │
│ │ Load JSON...     │ │ │ Gold: 99999  │ │ │ Live viewer   │ │
│ │ List (1234 strs) │ │ │ Items...     │ │ │ Party         │ │
│ │ ▸ Search/Filter  │ │ │ Switches     │ │ │ Switches      │ │
│ │ ▸ Source groups  │ │ │ Variables    │ │ │ Variables     │ │
│ │ ▸ Translation %  │ │ │ [Victory!]   │ │ │ Map/Scene     │ │
│ └──────────────────┘ │ └──────────────┘ │ └───────────────┘ │
├──────────────────────┴──────────────────┴───────────────────┤
│ ● Connected | Map 12 | 1,234 strings | 45% translated       │  ← Status Bar
└─────────────────────────────────────────────────────────────┘
```

### 7.2 Panels

- **Translation Panel (left):** Load/manage translation JSONs, view/extract/export strings, built-in editor
- **Modification Panel (center):** Gold editor, item picker, switch/variable toggles, one-click victory button
- **Game State Panel (right):** Live readout of party, switches, variables, current map
- **Status Bar (bottom):** Connection indicator, game info, translation coverage stats

### 7.3 Technology

- **Qt 6 + QML** for the desktop application shell
- **Qt WebSockets** module for WS server
- **Qt Network** for CDP HTTP/WS client
- **Qt Quick Controls 2** for UI components

---

## 8. Error Handling & Edge Cases

### 8.1 Launch Failures

| Scenario | Detection | Response |
|----------|-----------|----------|
| Game exe not found | File check before spawn | Show file picker, error badge |
| Not an RPG Maker game | Check for www/data/System.json | Warn, allow force or cancel |
| CDP port in use | Port scan before launch | Try port+1 up to 5 times |
| CDP connection timeout | 3-second timeout | "Injection failed", game runs uninstrumented |
| Game encrypted/obfuscated | JSON parse failure | Translation disabled, cheat-only mode |

### 8.2 Runtime Disconnections

- **Game crash:** WS disconnect → Qt shows "disconnected" → retry 3x → offer "Relaunch"
- **Qt crash:** Hook script enters standalone mode → translation continues (map in memory) → cheats unavailable
- **Slow game:** Callback-driven state updates (push on scene change, map change, battle events), no polling

### 8.3 Translation Edge Cases

- **Escape codes:** Match full string including codes; never strip them
- **Multi-line messages:** Hook at `Game_Message.add` level before rendering splits
- **Dynamic text templates:** Match the template pattern (`"You have \\V[42] gold"`), not the evaluated result

---

## 9. Project Structure

```
Rtranslator_rebuild/
├── CMakeLists.txt                  # Top-level CMake
├── src/
│   ├── main.cpp                    # Entry point
│   ├── app/
│   │   ├── Application.h/cpp       # Qt app setup
│   │   └── MainWindow.h/cpp        # QML bridge
│   ├── core/
│   │   ├── GameManager.h/cpp       # Game launch/lifecycle
│   │   ├── TranslationEngine.h/cpp # Text extraction, map building
│   │   └── CheatController.h/cpp   # Read/write/batch operations
│   ├── bridge/
│   │   ├── CdpClient.h/cpp         # CDP connection + injection
│   │   ├── WebSocketServer.h/cpp   # WS server for game comm
│   │   └── Protocol.h/cpp          # JSON message serialization
│   ├── adapter/
│   │   ├── IGameAdapter.h          # Abstract game engine interface
│   │   ├── RpgMVAdapter.h/cpp      # MV-specific data parsing
│   │   └── RpgMZAdapter.h/cpp      # MZ-specific data parsing
│   └── util/
│       ├── JsonHelper.h/cpp        # JSON utilities
│       └── PathResolver.h/cpp      # Dot-path to JSON path mapping
├── qml/
│   ├── main.qml                    # Root QML window
│   ├── TopBar.qml                  # Game selection + controls
│   ├── TranslationPanel.qml        # Translation workflow panel
│   ├── ModificationPanel.qml       # Cheat controls panel
│   ├── GameStatePanel.qml          # Live state viewer
│   ├── StatusBar.qml               # Connection/game status
│   └── components/                 # Reusable QML components
├── inject/                         # JS hook scripts (injected into game)
│   ├── hook.js                     # Main hook: text interceptor, state R/W
│   ├── translation.js              # Translation map lookup + replacement
│   ├── state-reader.js             # Game state path accessor
│   └── ws-client.js                # WebSocket client logic
└── tests/
    ├── test_translation_engine.cpp
    ├── test_cheat_controller.cpp
    ├── test_protocol.cpp
    └── test_data/                   # Sample RPG Maker data for testing
```

---

## 10. Testing Strategy

### 10.1 Unit Tests (C++ / Google Test)

- `TranslationEngine` — text extraction from sample JSON files, whitelist field correctness, dedup logic
- `CheatController` — path resolution, read/write serialization, batch atomicity
- `Protocol` — JSON message serialization/deserialization round-trip
- `JsonHelper` — JSON parsing edge cases, UTF-8 handling

### 10.2 Integration Tests

- Launch a minimal MV/MZ test project, inject hook, verify translation is applied
- Launch a test project, perform read/write/batch operations, verify correctness
- Simulate CDP connection failure and recovery
- Simulate WebSocket disconnection and standalone mode

### 10.3 Manual QA

- Test against real RPG Maker MV/MZ games (sample projects)
- Verify translation coverage across all UI types (menus, messages, choices, battle)
- Verify modification features work across scene transitions
- Test with encrypted/obfuscated games

### 10.4 JS Hook Tests

- Unit test hook functions in Node.js with mocked RPG Maker globals
- Integration test with actual NW.js instance

---

## 11. Dependencies

| Dependency | Purpose | License |
|-----------|---------|---------|
| Qt 6.5+ (Core, Quick, WebSockets, Network) | UI framework + IPC | LGPL |
| nlohmann/json | JSON parsing in C++ | MIT |
| Google Test | Unit testing | BSD |
| CMake 3.20+ | Build system | BSD |

---

## 12. Open Questions / Future Work

- **Plugin text support:** How to extract text from common MV/MZ plugins without per-plugin configuration
- **RTL language support:** Arabic, Hebrew text rendering may need game-side font patching
- **Font injection:** CJK games might need CJK fonts injected for translation rendering
- **Multi-language switching:** Hot-swap between multiple translation files at runtime
- **Auto-translate:** Integration with machine translation APIs for rapid prototyping
- **XP/VX Ace support:** Ruby runtime injection via RGSS hooks (separate adapter)
