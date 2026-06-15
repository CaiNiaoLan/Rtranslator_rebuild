# RTranslator Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a C++ Qt desktop app that translates and modifies RPG Maker MV/MZ games at runtime via CDP injection + WebSocket IPC.

**Architecture:** Qt 6 QML UI launches the NW.js game with `--remote-debugging-port`, injects JS hooks via Chrome DevTools Protocol, then communicates bidirectionally through a local WebSocket for translation lookup and game-state read/write.

**Tech Stack:** C++17, Qt 6.5+ (Core/Quick/WebSockets/Network), nlohmann/json, Google Test, CMake 3.20+

---

## Phase 1: Foundation

### Task 1: Project Skeleton & CMake Setup

**Files:**
- Create: `CMakeLists.txt`
- Create: `src/main.cpp`
- Create: `tests/CMakeLists.txt`
- Create: `tests/test_main.cpp`

- [ ] **Step 1: Write top-level CMakeLists.txt**

```cmake
cmake_minimum_required(VERSION 3.20)
project(RTranslator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick WebSockets Network)

include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
  )
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  add_subdirectory(tests)
endif()

file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.h)

add_executable(RTranslator ${SRC_FILES} qml/qml.qrc)

target_include_directories(RTranslator PRIVATE src)
target_link_libraries(RTranslator PRIVATE
  Qt6::Core Qt6::Quick Qt6::WebSockets Qt6::Network
  nlohmann_json::nlohmann_json
)

set_target_properties(RTranslator PROPERTIES
  WIN32_EXECUTABLE TRUE
  MACOSX_BUNDLE TRUE
)

install(TARGETS RTranslator DESTINATION bin)
```

- [ ] **Step 2: Create directory structure**

```bash
New-Item -ItemType Directory -Force -Path src/app, src/core, src/bridge, src/adapter, src/util
New-Item -ItemType Directory -Force -Path qml/components, inject, tests/test_data
```

- [ ] **Step 3: Write minimal main.cpp**

Create `src/main.cpp`:
```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QDir>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("RTranslator");
    app.setApplicationName("RTranslator");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
```

- [ ] **Step 4: Create QML resource file**

Create `qml/qml.qrc`:
```xml
<RCC>
    <qresource prefix="/qml">
        <file>main.qml</file>
    </qresource>
</RCC>
```

- [ ] **Step 5: Write placeholder main.qml**

Create `qml/main.qml`:
```qml
import QtQuick
import QtQuick.Window

Window {
    width: 1280
    height: 800
    visible: true
    title: "RTranslator"
    color: "#1e1e2e"

    Text {
        anchors.centerIn: parent
        text: "RTranslator"
        color: "#cdd6f4"
        font.pixelSize: 32
    }
}
```

- [ ] **Step 6: Write test CMakeLists.txt**

Create `tests/CMakeLists.txt`:
```cmake
add_executable(rtranslator_tests test_main.cpp)
target_include_directories(rtranslator_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(rtranslator_tests PRIVATE
  GTest::gtest_main
  nlohmann_json::nlohmann_json
  Qt6::Core Qt6::WebSockets Qt6::Network
)
include(GoogleTest)
gtest_discover_tests(rtranslator_tests)
```

- [ ] **Step 7: Write test_main.cpp stub**

Create `tests/test_main.cpp`:
```cpp
#include <gtest/gtest.h>

TEST(RTranslator, SanityCheck) {
    EXPECT_EQ(1, 1);
}
```

- [ ] **Step 8: Configure and build**

```bash
cmake -B build -S .
cmake --build build --config Debug
```

- [ ] **Step 9: Run tests to verify**

```bash
cmake --build build --config Debug --target test
```

- [ ] **Step 10: Commit**

```bash
git add -A
git commit -m "chore: project skeleton with CMake, Qt6, GTest, nlohmann/json"
```

---

### Task 2: Protocol — Message Types & Serialization

**Files:**
- Create: `src/bridge/Protocol.h`
- Create: `src/bridge/Protocol.cpp`
- Modify: `tests/CMakeLists.txt`
- Create: `tests/test_protocol.cpp`

- [ ] **Step 1: Write the failing test**

Create `tests/test_protocol.cpp`:
```cpp
#include <gtest/gtest.h>
#include "bridge/Protocol.h"

using json = nlohmann::json;

TEST(Protocol, InitCommand_serializes_correctly) {
    Protocol::InitCommand cmd;
    cmd.translationMap = {{"hello", "nihao"}, {"world", "shijie"}};
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "init");
    EXPECT_TRUE(j.contains("map"));
    EXPECT_EQ(j["map"]["hello"], "nihao");
    EXPECT_EQ(j["map"]["world"], "shijie");
}

TEST(Protocol, ReadCommand_serializes_path) {
    Protocol::ReadCommand cmd;
    cmd.path = "party.gold";
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "read");
    EXPECT_EQ(j["path"], "party.gold");
}

TEST(Protocol, WriteCommand_serializes_path_and_value) {
    Protocol::WriteCommand cmd;
    cmd.path = "party.gold";
    cmd.value = 99999;
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "write");
    EXPECT_EQ(j["path"], "party.gold");
    EXPECT_EQ(j["value"], 99999);
}

TEST(Protocol, parseStateResponse_extracts_path_and_value) {
    json j = {{"type", "state"}, {"path", "party.gold"}, {"value", 500}};
    auto resp = Protocol::StateResponse::fromJson(j);
    EXPECT_TRUE(resp.has_value());
    EXPECT_EQ(resp->path, "party.gold");
    EXPECT_EQ(resp->value, 500);
}

TEST(Protocol, parseAckResponse_extracts_success) {
    json j = {{"type", "ack"}, {"cmd", "write"}, {"success", true}};
    auto resp = Protocol::AckResponse::fromJson(j);
    EXPECT_TRUE(resp.has_value());
    EXPECT_TRUE(resp->success);
}

TEST(Protocol, parseErrorResponse_extracts_reason) {
    json j = {{"type", "error"}, {"cmd", "read"}, {"reason", "invalid path"}};
    auto resp = Protocol::ErrorResponse::fromJson(j);
    EXPECT_TRUE(resp.has_value());
    EXPECT_EQ(resp->reason, "invalid path");
}

TEST(Protocol, unknown_message_type_returns_nullopt) {
    json j = {{"type", "bogus"}};
    auto state = Protocol::StateResponse::fromJson(j);
    EXPECT_FALSE(state.has_value());
}

TEST(Protocol, BatchCommand_serializes_multiple_ops) {
    Protocol::BatchCommand batch;
    batch.ops.push_back(Protocol::ReadCommand{"party.gold"}.toJson());
    batch.ops.push_back(Protocol::WriteCommand{"switches.5", true}.toJson());
    json j = batch.toJson();
    EXPECT_EQ(j["type"], "batch");
    EXPECT_EQ(j["ops"].size(), 2);
    EXPECT_EQ(j["ops"][0]["type"], "read");
    EXPECT_EQ(j["ops"][1]["type"], "write");
}

TEST(Protocol, BattleVictoryCommand_serializes_empty_payload) {
    Protocol::BattleVictoryCommand cmd;
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "battle_victory");
}

TEST(Protocol, AddItemCommand_serializes_item_info) {
    Protocol::AddItemCommand cmd;
    cmd.id = 10;
    cmd.itemType = "item";
    cmd.count = 5;
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "add_item");
    EXPECT_EQ(j["id"], 10);
    EXPECT_EQ(j["itemType"], "item");
    EXPECT_EQ(j["count"], 5);
}

TEST(Protocol, ReloadTransCommand_serializes_new_map) {
    Protocol::ReloadTransCommand cmd;
    cmd.translationMap = {{"a", "b"}};
    json j = cmd.toJson();
    EXPECT_EQ(j["type"], "reload_trans");
    EXPECT_EQ(j["map"]["a"], "b");
}
```

- [ ] **Step 2: Run test to verify it fails**

```bash
cmake --build build --config Debug
cmake --build build --config Debug --target test
```
Expected: compilation failure — Protocol.h not found.

- [ ] **Step 3: Write Protocol.h**

Create `src/bridge/Protocol.h`:
```cpp
#pragma once
#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace Protocol {

using json = nlohmann::json;
using TranslationMap = std::unordered_map<std::string, std::string>;

struct InitCommand {
    TranslationMap translationMap;
    json toJson() const;
};

struct ReadCommand {
    std::string path;
    json toJson() const;
};

struct WriteCommand {
    std::string path;
    json value;
    json toJson() const;
};

struct BatchCommand {
    std::vector<json> ops;
    json toJson() const;
};

struct BattleVictoryCommand {
    json toJson() const;
};

struct AddItemCommand {
    int id;
    std::string itemType; // "item", "weapon", "armor"
    int count;
    json toJson() const;
};

struct ReloadTransCommand {
    TranslationMap translationMap;
    json toJson() const;
};

struct StateResponse {
    std::string path;
    json value;
    static std::optional<StateResponse> fromJson(const json& j);
};

struct AckResponse {
    std::string cmd;
    bool success;
    static std::optional<AckResponse> fromJson(const json& j);
};

struct ErrorResponse {
    std::string cmd;
    std::string reason;
    static std::optional<ErrorResponse> fromJson(const json& j);
};

} // namespace Protocol
```

- [ ] **Step 4: Write Protocol.cpp**

Create `src/bridge/Protocol.cpp`:
```cpp
#include "bridge/Protocol.h"

namespace Protocol {

json InitCommand::toJson() const {
    return {{"type", "init"}, {"map", translationMap}};
}

json ReadCommand::toJson() const {
    return {{"type", "read"}, {"path", path}};
}

json WriteCommand::toJson() const {
    return {{"type", "write"}, {"path", path}, {"value", value}};
}

json BatchCommand::toJson() const {
    return {{"type", "batch"}, {"ops", ops}};
}

json BattleVictoryCommand::toJson() const {
    return {{"type", "battle_victory"}};
}

json AddItemCommand::toJson() const {
    return {{"type", "add_item"}, {"id", id}, {"itemType", itemType}, {"count", count}};
}

json ReloadTransCommand::toJson() const {
    return {{"type", "reload_trans"}, {"map", translationMap}};
}

std::optional<StateResponse> StateResponse::fromJson(const json& j) {
    if (!j.contains("type") || j["type"] != "state") return std::nullopt;
    if (!j.contains("path") || !j.contains("value")) return std::nullopt;
    return StateResponse{j["path"].get<std::string>(), j["value"]};
}

std::optional<AckResponse> AckResponse::fromJson(const json& j) {
    if (!j.contains("type") || j["type"] != "ack") return std::nullopt;
    return AckResponse{j.value("cmd", ""), j.value("success", false)};
}

std::optional<ErrorResponse> ErrorResponse::fromJson(const json& j) {
    if (!j.contains("type") || j["type"] != "error") return std::nullopt;
    return ErrorResponse{j.value("cmd", ""), j.value("reason", "unknown")};
}

} // namespace Protocol
```

- [ ] **Step 5: Run tests to verify they pass**

```bash
cmake --build build --config Debug
cmake --build build --config Debug --target test
```
Expected: All Protocol tests PASS.

- [ ] **Step 6: Commit**

```bash
git add src/bridge/Protocol.h src/bridge/Protocol.cpp tests/test_protocol.cpp
git commit -m "feat: add Protocol message types and serialization"
```

---

### Task 3: JSON Helper Utilities

**Files:**
- Create: `src/util/JsonHelper.h`
- Create: `src/util/JsonHelper.cpp`
- Create: `tests/test_json_helper.cpp`

- [ ] **Step 1: Write the failing test**

Create `tests/test_json_helper.cpp`:
```cpp
#include <gtest/gtest.h>
#include "util/JsonHelper.h"

using json = nlohmann::json;

TEST(JsonHelper, loadJsonFile_reads_valid_file) {
    auto opt = JsonHelper::loadJsonFile("tests/test_data/valid.json");
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ((*opt)["key"], "value");
}

TEST(JsonHelper, loadJsonFile_returns_nullopt_for_missing_file) {
    auto opt = JsonHelper::loadJsonFile("tests/test_data/does_not_exist.json");
    EXPECT_FALSE(opt.has_value());
}

TEST(JsonHelper, loadJsonFile_returns_nullopt_for_invalid_json) {
    auto opt = JsonHelper::loadJsonFile("tests/test_data/invalid.json");
    EXPECT_FALSE(opt.has_value());
}

TEST(JsonHelper, getString_returns_value_when_field_exists) {
    json obj = {{"name", "Hero"}};
    auto val = JsonHelper::getString(obj, "name");
    EXPECT_TRUE(val.has_value());
    EXPECT_EQ(*val, "Hero");
}

TEST(JsonHelper, getString_returns_nullopt_when_field_missing) {
    json obj = {{"name", "Hero"}};
    auto val = JsonHelper::getString(obj, "description");
    EXPECT_FALSE(val.has_value());
}

TEST(JsonHelper, getString_returns_nullopt_for_non_string_type) {
    json obj = {{"count", 42}};
    auto val = JsonHelper::getString(obj, "count");
    EXPECT_FALSE(val.has_value());
}

TEST(JsonHelper, collectStrings_aggregates_from_array_field) {
    json arr = json::array({
        {{"name", "A"}},
        {{"name", "B"}},
        {{"name", "A"}}  // duplicate
    });
    std::vector<std::string> result;
    JsonHelper::collectStrings(arr, "name", result);
    EXPECT_EQ(result.size(), 3);
}

TEST(JsonHelper, collectStringsToSet_dedupicates) {
    json arr = json::array({
        {{"name", "A"}},
        {{"name", "B"}},
        {{"name", "A"}}
    });
    std::unordered_set<std::string> result;
    JsonHelper::collectStringsToSet(arr, "name", result);
    EXPECT_EQ(result.size(), 2);
}
```

- [ ] **Step 2: Create test data files**

Create `tests/test_data/valid.json`:
```json
{"key": "value"}
```

Create `tests/test_data/invalid.json`:
```
this is not valid json
```

- [ ] **Step 3: Write JsonHelper.h**

Create `src/util/JsonHelper.h`:
```cpp
#pragma once
#include <optional>
#include <string>
#include <vector>
#include <unordered_set>
#include <nlohmann/json.hpp>

namespace JsonHelper {

using json = nlohmann::json;

std::optional<json> loadJsonFile(const std::string& filepath);
std::optional<std::string> getString(const json& obj, const std::string& key);
void collectStrings(const json& arr, const std::string& key, std::vector<std::string>& out);
void collectStringsToSet(const json& arr, const std::string& key, std::unordered_set<std::string>& out);

} // namespace JsonHelper
```

- [ ] **Step 4: Write JsonHelper.cpp**

Create `src/util/JsonHelper.cpp`:
```cpp
#include "util/JsonHelper.h"
#include <fstream>
#include <QFile>
#include <QTextStream>

namespace JsonHelper {

std::optional<json> loadJsonFile(const std::string& filepath) {
    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return std::nullopt;
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString content = stream.readAll();
    file.close();
    try {
        return json::parse(content.toStdString());
    } catch (const json::parse_error&) {
        return std::nullopt;
    }
}

std::optional<std::string> getString(const json& obj, const std::string& key) {
    auto it = obj.find(key);
    if (it == obj.end() || !it->is_string())
        return std::nullopt;
    return it->get<std::string>();
}

void collectStrings(const json& arr, const std::string& key, std::vector<std::string>& out) {
    if (!arr.is_array()) return;
    for (const auto& item : arr) {
        auto val = getString(item, key);
        if (val.has_value() && !val->empty())
            out.push_back(*val);
    }
}

void collectStringsToSet(const json& arr, const std::string& key, std::unordered_set<std::string>& out) {
    if (!arr.is_array()) return;
    for (const auto& item : arr) {
        auto val = getString(item, key);
        if (val.has_value() && !val->empty())
            out.insert(*val);
    }
}

} // namespace JsonHelper
```

- [ ] **Step 5: Run tests to verify they pass**

```bash
cmake --build build --config Debug
cmake --build build --config Debug --target test
```

- [ ] **Step 6: Commit**

```bash
git add src/util/JsonHelper.h src/util/JsonHelper.cpp tests/test_json_helper.cpp tests/test_data/
git commit -m "feat: add JsonHelper for JSON file loading and field extraction"
```

---

## Phase 2: Game Data Adapter & Translation Engine

### Task 4: IGameAdapter Interface & Game Detection

**Files:**
- Create: `src/adapter/IGameAdapter.h`
- Create: `src/adapter/RpgMVAdapter.h`
- Create: `src/adapter/RpgMVAdapter.cpp`
- Create: `tests/test_rpgmv_adapter.cpp`

- [ ] **Step 1: Write IGameAdapter.h**

Create `src/adapter/IGameAdapter.h`:
```cpp
#pragma once
#include <string>
#include <vector>
#include <optional>

struct TranslationEntry {
    std::string id;       // e.g. "Actors/1/name"
    std::string source;   // e.g. "Actors.json"
    std::string context;  // e.g. "Actor #1 - Name"
    std::string field;    // e.g. "name"
    std::string original; // the source-language text
    std::string translation; // empty until user fills
};

class IGameAdapter {
public:
    virtual ~IGameAdapter() = default;

    // Check if the given directory is a supported game
    virtual bool detect(const std::string& gameDir) = 0;

    // Extract all translatable text from the game data
    virtual std::vector<TranslationEntry> extractText(const std::string& gameDir) = 0;

    // Get the executable path to launch (relative to gameDir or absolute)
    virtual std::string executableName() const = 0;

    // Get engine display name
    virtual std::string engineName() const = 0;

    // Get the port argument flag for CDP (e.g. "--remote-debugging-port=")
    virtual std::string debugPortFlag() const { return "--remote-debugging-port="; }
};
```

- [ ] **Step 2: Write RpgMVAdapter.h**

Create `src/adapter/RpgMVAdapter.h`:
```cpp
#pragma once
#include "adapter/IGameAdapter.h"

class RpgMVAdapter : public IGameAdapter {
public:
    bool detect(const std::string& gameDir) override;
    std::vector<TranslationEntry> extractText(const std::string& gameDir) override;
    std::string executableName() const override { return "Game.exe"; }
    std::string engineName() const override { return "RPG Maker MV"; }

private:
    void extractFromFile(const std::string& filePath, const std::string& source,
                         const std::string& contextPrefix,
                         const std::vector<std::string>& fields,
                         std::vector<TranslationEntry>& out);
    void extractFromMapFile(const std::string& filePath, const std::string& source,
                            std::vector<TranslationEntry>& out);
    void extractFromSystemFile(const std::string& filePath, const std::string& source,
                               std::vector<TranslationEntry>& out);
    void extractEventParameters(const nlohmann::json& list, const std::string& source,
                                const std::string& context,
                                std::vector<TranslationEntry>& out);
};
```

- [ ] **Step 3: Write RpgMVAdapter.cpp**

Create `src/adapter/RpgMVAdapter.cpp`:
```cpp
#include "adapter/RpgMVAdapter.h"
#include "util/JsonHelper.h"
#include <QDir>
#include <QFileInfo>
#include <filesystem>

namespace fs = std::filesystem;

bool RpgMVAdapter::detect(const std::string& gameDir) {
    QDir dir(QString::fromStdString(gameDir));
    return dir.exists("www/data/System.json");
}

std::vector<TranslationEntry> RpgMVAdapter::extractText(const std::string& gameDir) {
    std::vector<TranslationEntry> entries;
    std::string dataPath = gameDir + "/www/data";

    // Data-driven file definitions: {filename, contextPrefix, translatableFields}
    struct FileSpec { std::string name; std::string prefix; std::vector<std::string> fields; };
    std::vector<FileSpec> specs = {
        {"Actors.json",   "Actor",   {"name", "nickname", "profile", "note"}},
        {"Classes.json",  "Class",   {"name", "note"}},  // description handled as multi-field
        {"Skills.json",   "Skill",   {"name", "description", "note", "message1", "message2"}},
        {"Items.json",    "Item",    {"name", "description", "note", "message1", "message2"}},
        {"Weapons.json",  "Weapon",  {"name", "description", "note", "message1", "message2"}},
        {"Armors.json",   "Armor",   {"name", "description", "note", "message1", "message2"}},
        {"Enemies.json",  "Enemy",   {"name", "note"}},
        {"States.json",   "State",   {"name", "message1", "message2", "message3", "message4", "note"}},
        {"Troops.json",   "Troop",   {"name"}},
        {"Animations.json","Animation", {"name"}},
        {"Tilesets.json", "Tileset", {"name", "note"}},
    };

    for (const auto& spec : specs) {
        std::string filePath = dataPath + "/" + spec.name;
        if (!QFileInfo::exists(QString::fromStdString(filePath))) continue;
        extractFromFile(filePath, spec.name, spec.prefix, spec.fields, entries);
    }

    // System.json — special handling for nested terms
    std::string systemPath = dataPath + "/System.json";
    if (QFileInfo::exists(QString::fromStdString(systemPath))) {
        extractFromSystemFile(systemPath, "System.json", entries);
    }

    // CommonEvents.json
    std::string commonPath = dataPath + "/CommonEvents.json";
    if (QFileInfo::exists(QString::fromStdString(commonPath))) {
        auto opt = JsonHelper::loadJsonFile(commonPath);
        if (opt.has_value() && opt->is_array()) {
            int idx = 0;
            for (const auto& event : *opt) {
                if (event.contains("name")) {
                    auto name = JsonHelper::getString(event, "name");
                    if (name.has_value() && !name->empty()) {
                        entries.push_back({"CommonEvents/" + std::to_string(idx) + "/name",
                            "CommonEvents.json", "Common Event #" + std::to_string(idx) + " - Name",
                            "name", *name, ""});
                    }
                }
                if (event.contains("list")) {
                    extractEventParameters(event["list"], "CommonEvents.json",
                        "CommonEvent #" + std::to_string(idx), entries);
                }
                idx++;
            }
        }
    }

    // MapXXX.json files
    for (int i = 1; i <= 999; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "/Map%03d.json", i);
        std::string mapPath = dataPath + buf;
        if (QFileInfo::exists(QString::fromStdString(mapPath))) {
            extractFromMapFile(mapPath, std::string("Map") + buf, entries);
        }
    }

    return entries;
}

void RpgMVAdapter::extractFromFile(const std::string& filePath, const std::string& source,
                                    const std::string& contextPrefix,
                                    const std::vector<std::string>& fields,
                                    std::vector<TranslationEntry>& out) {
    auto opt = JsonHelper::loadJsonFile(filePath);
    if (!opt.has_value() || !opt->is_array()) return;

    int idx = 0;
    for (const auto& item : *opt) {
        if (item.is_null()) { idx++; continue; }
        for (const auto& field : fields) {
            auto val = JsonHelper::getString(item, field);
            if (val.has_value() && !val->empty()) {
                out.push_back({
                    source + "/" + std::to_string(idx) + "/" + field,
                    source,
                    contextPrefix + " #" + std::to_string(idx) + " - " + field,
                    field,
                    *val,
                    ""
                });
            }
        }
        idx++;
    }
}

void RpgMVAdapter::extractFromMapFile(const std::string& filePath, const std::string& source,
                                       std::vector<TranslationEntry>& out) {
    auto opt = JsonHelper::loadJsonFile(filePath);
    if (!opt.has_value()) return;

    // Map display name
    if (opt->contains("displayName")) {
        auto name = JsonHelper::getString(*opt, "displayName");
        if (name.has_value() && !name->empty()) {
            out.push_back({source + "/displayName", source, "Map Display Name", "displayName", *name, ""});
        }
    }

    // Events
    if (opt->contains("events") && (*opt)["events"].is_array()) {
        for (const auto& event : (*opt)["events"]) {
            if (event.is_null()) continue;
            std::string eventName = event.value("name", "");
            int eventId = event.value("id", 0);

            // Event name
            if (!eventName.empty()) {
                out.push_back({source + "/event" + std::to_string(eventId) + "/name",
                    source, "Event #" + std::to_string(eventId) + " Name", "name", eventName, ""});
            }

            if (event.contains("pages")) {
                for (const auto& page : event["pages"]) {
                    if (page.is_null() || !page.contains("list")) continue;
                    int pageIdx = page.value("pageIndex", 0);
                    std::string ctx = "Event #" + std::to_string(eventId) + " Page " + std::to_string(pageIdx);
                    extractEventParameters(page["list"], source, ctx, out);
                }
            }
        }
    }
}

void RpgMVAdapter::extractEventParameters(const nlohmann::json& list, const std::string& source,
                                           const std::string& context,
                                           std::vector<TranslationEntry>& out) {
    if (!list.is_array()) return;
    int cmdIdx = 0;
    for (const auto& cmd : list) {
        if (!cmd.contains("code") || !cmd.contains("parameters")) {
            cmdIdx++; continue;
        }
        int code = cmd["code"].get<int>();
        const auto& params = cmd["parameters"];

        // 101 = Show Text, 102 = Show Choices, 401 = Show Text (continuation)
        if (code == 101 || code == 401) {
            if (params.is_array() && params.size() > 0 && params[0].is_string()) {
                std::string text = params[0].get<std::string>();
                if (!text.empty()) {
                    out.push_back({source + "/cmd" + std::to_string(cmdIdx),
                        source, context + " - Message", "message", text, ""});
                }
            }
        }
        // 102 = Show Choices
        else if (code == 102) {
            if (params.is_array()) {
                for (size_t pi = 0; pi < params.size(); pi++) {
                    if (params[pi].is_string()) {
                        std::string choice = params[pi].get<std::string>();
                        if (!choice.empty()) {
                            out.push_back({source + "/cmd" + std::to_string(cmdIdx) + "/choice" + std::to_string(pi),
                                source, context + " - Choice #" + std::to_string(pi), "choice", choice, ""});
                        }
                    }
                }
            }
        }
        cmdIdx++;
    }
}

void RpgMVAdapter::extractFromSystemFile(const std::string& filePath, const std::string& source,
                                          std::vector<TranslationEntry>& out) {
    auto opt = JsonHelper::loadJsonFile(filePath);
    if (!opt.has_value()) return;

    json& sys = *opt;

    // Top-level fields
    auto pushSysField = [&](const std::string& field) {
        auto val = JsonHelper::getString(sys, field);
        if (val.has_value() && !val->empty()) {
            out.push_back({source + "/" + field, source, "System - " + field, field, *val, ""});
        }
    };
    pushSysField("gameTitle");
    pushSysField("currencyUnit");

    // Elements array
    if (sys.contains("elements") && sys["elements"].is_array()) {
        int i = 0;
        for (const auto& elem : sys["elements"]) {
            if (elem.is_string()) {
                std::string s = elem.get<std::string>();
                if (!s.empty()) {
                    out.push_back({source + "/elements/" + std::to_string(i),
                        source, "System - Element #" + std::to_string(i), "element", s, ""});
                }
            }
            i++;
        }
    }

    // Terms sub-object — flatten all string values
    if (sys.contains("terms") && sys["terms"].is_object()) {
        for (auto& [termKey, termVal] : sys["terms"].items()) {
            if (termVal.is_string()) {
                std::string s = termVal.get<std::string>();
                if (!s.empty()) {
                    out.push_back({source + "/terms/" + termKey,
                        source, "System - Term: " + termKey, "term", s, ""});
                }
            } else if (termVal.is_array()) {
                for (size_t i = 0; i < termVal.size(); i++) {
                    if (termVal[i].is_string()) {
                        std::string s = termVal[i].get<std::string>();
                        if (!s.empty()) {
                            out.push_back({source + "/terms/" + termKey + "/" + std::to_string(i),
                                source, "System - Term: " + termKey + "[" + std::to_string(i) + "]", "term", s, ""});
                        }
                    }
                }
            }
        }
    }
}
```

- [ ] **Step 4: Write the adapter test**

Create `tests/test_rpgmv_adapter.cpp`:
```cpp
#include <gtest/gtest.h>
#include "adapter/RpgMVAdapter.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

class RpgMVAdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        testDir = "tests/test_data/mv_game/www/data";
        QDir().mkpath(testDir);
    }

    void writeJson(const std::string& filename, const std::string& content) {
        QFile file(QString::fromStdString(testDir + "/" + filename));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out.setCodec("UTF-8");
        out << QString::fromStdString(content);
        file.close();
    }

    void TearDown() override {
        QDir("tests/test_data/mv_game").removeRecursively();
    }

    std::string testDir;
};

TEST_F(RpgMVAdapterTest, detect_returns_false_when_no_system_json) {
    RpgMVAdapter adapter;
    EXPECT_FALSE(adapter.detect("tests/test_data/mv_game"));
}

TEST_F(RpgMVAdapterTest, detect_returns_true_when_system_json_exists) {
    writeJson("System.json", "{}");
    RpgMVAdapter adapter;
    EXPECT_TRUE(adapter.detect("tests/test_data/mv_game"));
}

TEST_F(RpgMVAdapterTest, extractText_parses_actors) {
    writeJson("System.json", "{}");
    writeJson("Actors.json", R"([{"id":1,"name":"Hero","nickname":"H","profile":"Brave","note":"Tag"}])");

    RpgMVAdapter adapter;
    auto entries = adapter.extractText("tests/test_data/mv_game");
    EXPECT_GE(entries.size(), 4);
}

TEST_F(RpgMVAdapterTest, extractText_parses_items) {
    writeJson("System.json", "{}");
    writeJson("Items.json", R"([{"id":1,"name":"Potion","description":"Heals 50HP","note":"Common"}])");

    RpgMVAdapter adapter;
    auto entries = adapter.extractText("tests/test_data/mv_game");
    EXPECT_GE(entries.size(), 3);
}

TEST_F(RpgMVAdapterTest, extractText_parses_system_terms) {
    writeJson("System.json", R"({
        "gameTitle":"MyRPG",
        "currencyUnit":"G",
        "terms":{"basic":["HP","MP"],"commands":["Fight","Escape"]}
    })");

    RpgMVAdapter adapter;
    auto entries = adapter.extractText("tests/test_data/mv_game");
    auto count = entries.size();
    EXPECT_GE(count, 5);
}
```

- [ ] **Step 5: Run tests to verify they pass**

```bash
cmake --build build --config Debug
cmake --build build --config Debug --target test
```

- [ ] **Step 6: Commit**

```bash
git add src/adapter/ tests/test_rpgmv_adapter.cpp
git commit -m "feat: add IGameAdapter interface and RpgMVAdapter"
```

---

### Task 5: TranslationEngine

**Files:**
- Create: `src/core/TranslationEngine.h`
- Create: `src/core/TranslationEngine.cpp`
- Create: `tests/test_translation_engine.cpp`

- [ ] **Step 1: Write TranslationEngine.h**

Create `src/core/TranslationEngine.h`:
```cpp
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "adapter/IGameAdapter.h"

class TranslationEngine {
public:
    explicit TranslationEngine(std::unique_ptr<IGameAdapter> adapter);

    // Extract all translatable text from a game directory
    std::vector<TranslationEntry> extractFromGame(const std::string& gameDir);

    // Load a translation JSON file into the runtime map
    bool loadTranslation(const std::string& filePath);

    // Get the translation for a given original string (empty string = not found)
    std::string translate(const std::string& original) const;

    // Get the runtime translation map (for sending to game hook)
    const std::unordered_map<std::string, std::string>& translationMap() const;

    // Check if a game is detected at the given path
    bool detectGame(const std::string& gameDir) const;

    // Access the adapter
    IGameAdapter* adapter() const;

    // Build the whitelist set from extracted entries
    std::unordered_set<std::string> buildWhitelist(const std::vector<TranslationEntry>& entries) const;

private:
    std::unique_ptr<IGameAdapter> m_adapter;
    std::unordered_map<std::string, std::string> m_translationMap;
    std::vector<TranslationEntry> m_entries;
};
```

- [ ] **Step 2: Write TranslationEngine.cpp**

Create `src/core/TranslationEngine.cpp`:
```cpp
#include "core/TranslationEngine.h"
#include "util/JsonHelper.h"

TranslationEngine::TranslationEngine(std::unique_ptr<IGameAdapter> adapter)
    : m_adapter(std::move(adapter)) {}

bool TranslationEngine::detectGame(const std::string& gameDir) const {
    return m_adapter->detect(gameDir);
}

IGameAdapter* TranslationEngine::adapter() const {
    return m_adapter.get();
}

std::vector<TranslationEntry> TranslationEngine::extractFromGame(const std::string& gameDir) {
    m_entries = m_adapter->extractText(gameDir);
    return m_entries;
}

bool TranslationEngine::loadTranslation(const std::string& filePath) {
    auto opt = JsonHelper::loadJsonFile(filePath);
    if (!opt.has_value()) return false;

    auto& doc = *opt;
    if (!doc.contains("strings") || !doc["strings"].is_object()) return false;

    m_translationMap.clear();
    for (auto& [key, value] : doc["strings"].items()) {
        if (value.is_string()) {
            m_translationMap[key] = value.get<std::string>();
        }
    }
    return true;
}

std::string TranslationEngine::translate(const std::string& original) const {
    auto it = m_translationMap.find(original);
    return (it != m_translationMap.end()) ? it->second : std::string();
}

const std::unordered_map<std::string, std::string>& TranslationEngine::translationMap() const {
    return m_translationMap;
}

std::unordered_set<std::string> TranslationEngine::buildWhitelist(
    const std::vector<TranslationEntry>& entries) const {
    std::unordered_set<std::string> whitelist;
    for (const auto& entry : entries) {
        whitelist.insert(entry.original);
    }
    return whitelist;
}
```

- [ ] **Step 3: Write tests**

Create `tests/test_translation_engine.cpp`:
```cpp
#include <gtest/gtest.h>
#include "core/TranslationEngine.h"
#include "adapter/RpgMVAdapter.h"
#include <QDir>
#include <QFile>
#include <QTextStream>

class TranslationEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        engine = std::make_unique<TranslationEngine>(std::make_unique<RpgMVAdapter>());
        testDir = "tests/test_data/trans_engine/www/data";
        QDir().mkpath(testDir);
    }

    void writeJson(const std::string& filename, const std::string& content) {
        QFile file(QString::fromStdString(testDir + "/" + filename));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << QString::fromStdString(content);
        file.close();
    }

    void TearDown() override {
        QDir("tests/test_data/trans_engine").removeRecursively();
    }

    std::unique_ptr<TranslationEngine> engine;
    std::string testDir;
};

TEST_F(TranslationEngineTest, detectGame_returns_true_for_valid_game) {
    writeJson("System.json", "{}");
    EXPECT_TRUE(engine->detectGame("tests/test_data/trans_engine"));
}

TEST_F(TranslationEngineTest, detectGame_returns_false_for_empty_dir) {
    EXPECT_FALSE(engine->detectGame("tests/test_data/trans_engine"));
}

TEST_F(TranslationEngineTest, extractFromGame_returns_entries) {
    writeJson("System.json", R"({"gameTitle":"Test","currencyUnit":"G","terms":{}})");
    writeJson("Actors.json", R"([{"id":1,"name":"Hero","nickname":"H","profile":"P","note":"N"}])");
    auto entries = engine->extractFromGame("tests/test_data/trans_engine");
    EXPECT_GT(entries.size(), 0);
}

TEST_F(TranslationEngineTest, buildWhitelist_contains_extracted_strings) {
    std::vector<TranslationEntry> entries = {
        {"id1","src","ctx","f","hello",""},
        {"id2","src","ctx","f","world",""}
    };
    auto wl = engine->buildWhitelist(entries);
    EXPECT_TRUE(wl.contains("hello"));
    EXPECT_TRUE(wl.contains("world"));
    EXPECT_EQ(wl.size(), 2);
}

TEST_F(TranslationEngineTest, loadTranslation_returns_false_for_missing_file) {
    EXPECT_FALSE(engine->loadTranslation("tests/test_data/nonexistent.json"));
}

TEST_F(TranslationEngineTest, translate_returns_translation_when_found) {
    EXPECT_TRUE(engine->loadTranslation("tests/test_data/sample_trans.json"));
    auto result = engine->translate("hello");
    EXPECT_EQ(result, "nihao");
}

TEST_F(TranslationEngineTest, translate_returns_empty_when_not_found) {
    EXPECT_TRUE(engine->loadTranslation("tests/test_data/sample_trans.json"));
    auto result = engine->translate("unknown_key");
    EXPECT_TRUE(result.empty());
}
```

- [ ] **Step 4: Create sample translation JSON**

Create `tests/test_data/sample_trans.json`:
```json
{"strings": {"hello": "nihao", "world": "shijie"}}
```

- [ ] **Step 5: Build and run tests**

```bash
cmake --build build --config Debug
cmake --build build --config Debug --target test
```

- [ ] **Step 6: Commit**

```bash
git add src/core/TranslationEngine.h src/core/TranslationEngine.cpp tests/test_translation_engine.cpp tests/test_data/sample_trans.json
git commit -m "feat: add TranslationEngine with text extraction and map loading"
```

---

## Phase 3: IPC Bridge

### Task 6: WebSocketServer

**Files:**
- Create: `src/bridge/WebSocketServer.h`
- Create: `src/bridge/WebSocketServer.cpp`
- Create: `tests/test_websocket_server.cpp`

- [ ] **Step 1: Write WebSocketServer.h**

Create `src/bridge/WebSocketServer.h`:
```cpp
#pragma once
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <functional>

class WebSocketServer : public QObject {
    Q_OBJECT
public:
    using MessageHandler = std::function<void(const QString& message)>;

    explicit WebSocketServer(QObject* parent = nullptr);
    ~WebSocketServer();

    bool start(quint16 port = 0); // 0 = auto-assign
    void stop();
    quint16 port() const;
    bool isListening() const;

    void sendMessage(const QString& message);
    void setMessageHandler(MessageHandler handler);

signals:
    void clientConnected();
    void clientDisconnected();
    void messageReceived(const QString& message);

private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString& message);
    void onSocketDisconnected();

private:
    QWebSocketServer* m_server;
    QWebSocket* m_client; // Single client (the injected hook)
    MessageHandler m_handler;
};
```

- [ ] **Step 2: Write WebSocketServer.cpp**

Create `src/bridge/WebSocketServer.cpp`:
```cpp
#include "bridge/WebSocketServer.h"

WebSocketServer::WebSocketServer(QObject* parent)
    : QObject(parent)
    , m_server(new QWebSocketServer("RTranslator", QWebSocketServer::NonSecureMode, this))
    , m_client(nullptr)
{
    connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
}

WebSocketServer::~WebSocketServer() {
    stop();
}

bool WebSocketServer::start(quint16 port) {
    if (!m_server->listen(QHostAddress::LocalHost, port))
        return false;
    return true;
}

void WebSocketServer::stop() {
    if (m_client) {
        m_client->close();
        m_client->deleteLater();
        m_client = nullptr;
    }
    m_server->close();
}

quint16 WebSocketServer::port() const {
    return m_server->serverPort();
}

bool WebSocketServer::isListening() const {
    return m_server->isListening();
}

void WebSocketServer::sendMessage(const QString& message) {
    if (m_client && m_client->state() == QAbstractSocket::ConnectedState) {
        m_client->sendTextMessage(message);
    }
}

void WebSocketServer::setMessageHandler(MessageHandler handler) {
    m_handler = std::move(handler);
}

void WebSocketServer::onNewConnection() {
    if (m_client) {
        // Reject duplicate connections
        auto* dup = m_server->nextPendingConnection();
        dup->close();
        dup->deleteLater();
        return;
    }
    m_client = m_server->nextPendingConnection();
    connect(m_client, &QWebSocket::textMessageReceived, this, &WebSocketServer::onTextMessageReceived);
    connect(m_client, &QWebSocket::disconnected, this, &WebSocketServer::onSocketDisconnected);
    emit clientConnected();
}

void WebSocketServer::onTextMessageReceived(const QString& message) {
    emit messageReceived(message);
    if (m_handler) m_handler(message);
}

void WebSocketServer::onSocketDisconnected() {
    m_client->deleteLater();
    m_client = nullptr;
    emit clientDisconnected();
}
```

- [ ] **Step 3: Write tests**

Create `tests/test_websocket_server.cpp`:
```cpp
#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QCoreApplication>
#include "bridge/WebSocketServer.h"

TEST(WebSocketServer, starts_on_auto_port) {
    WebSocketServer server;
    EXPECT_TRUE(server.start(0));
    EXPECT_GT(server.port(), 0);
    EXPECT_TRUE(server.isListening());
}

TEST(WebSocketServer, rejects_duplicate_connections) {
    WebSocketServer server;
    ASSERT_TRUE(server.start(0));

    QTcpSocket sock1;
    sock1.connectToHost(QHostAddress::LocalHost, server.port());
    sock1.waitForConnected(1000);
    // Not yet upgraded to WS, so no newConnection signal yet

    server.stop();
}

TEST(WebSocketServer, stop_closes_server) {
    WebSocketServer server;
    server.start(0);
    server.stop();
    EXPECT_FALSE(server.isListening());
}
```

- [ ] **Step 4: Run tests**

```bash
cmake --build build --config Debug
cmake --build build --config Debug --target test
```

- [ ] **Step 5: Commit**

```bash
git add src/bridge/WebSocketServer.h src/bridge/WebSocketServer.cpp tests/test_websocket_server.cpp
git commit -m "feat: add WebSocketServer for game hook communication"
```

---

### Task 7: CdpClient — Chrome DevTools Protocol

**Files:**
- Create: `src/bridge/CdpClient.h`
- Create: `src/bridge/CdpClient.cpp`
- Create: `tests/test_cdp_client.cpp`

- [ ] **Step 1: Write CdpClient.h**

Create `src/bridge/CdpClient.h`:
```cpp
#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QTimer>
#include <functional>

class CdpClient : public QObject {
    Q_OBJECT
public:
    explicit CdpClient(QObject* parent = nullptr);

    // Connect to CDP endpoint at host:port, return the page WebSocket URL
    void connectToDebugger(const QString& host, quint16 port);

    // Inject a script into the target page
    void injectScript(const QString& jsCode);

    // Check if connected
    bool isConnected() const;

    using DisconnectHandler = std::function<void()>;
    void setDisconnectHandler(DisconnectHandler handler);

signals:
    void connected();
    void disconnected();
    void injectionComplete();

private slots:
    void onHttpResponse(QNetworkReply* reply);
    void onWsConnected();
    void onWsDisconnected();
    void onWsTextMessage(const QString& message);
    void onConnectionTimeout();

private:
    void connectToPage(const QString& wsUrl);

    QNetworkAccessManager* m_nam;
    QWebSocket* m_ws;
    QTimer* m_timeout;
    QString m_host;
    quint16 m_port;
    int m_cmdId;
    DisconnectHandler m_disconnectHandler;
};
```

- [ ] **Step 2: Write CdpClient.cpp**

Create `src/bridge/CdpClient.cpp`:
```cpp
#include "bridge/CdpClient.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

CdpClient::CdpClient(QObject* parent)
    : QObject(parent)
    , m_nam(new QNetworkAccessManager(this))
    , m_ws(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_timeout(new QTimer(this))
    , m_port(0)
    , m_cmdId(0)
{
    m_timeout->setSingleShot(true);
    m_timeout->setInterval(3000);

    connect(m_ws, &QWebSocket::connected, this, &CdpClient::onWsConnected);
    connect(m_ws, &QWebSocket::disconnected, this, &CdpClient::onWsDisconnected);
    connect(m_ws, &QWebSocket::textMessageReceived, this, &CdpClient::onWsTextMessage);
    connect(m_timeout, &QTimer::timeout, this, &CdpClient::onConnectionTimeout);
}

void CdpClient::connectToDebugger(const QString& host, quint16 port) {
    m_host = host;
    m_port = port;

    // GET http://host:port/json to list available pages
    QUrl url(QString("http://%1:%2/json").arg(host).arg(port));
    QNetworkRequest req(url);
    auto* reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onHttpResponse(reply);
    });
    m_timeout->start();
}

void CdpClient::onHttpResponse(QNetworkReply* reply) {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit disconnected();
        return;
    }

    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        emit disconnected();
        return;
    }

    QJsonArray pages = doc.array();
    for (const auto& page : pages) {
        QJsonObject obj = page.toObject();
        QString type = obj.value("type").toString();
        if (type == "page") {
            QString wsUrl = obj.value("webSocketDebuggerUrl").toString();
            if (!wsUrl.isEmpty()) {
                m_timeout->stop();
                connectToPage(wsUrl);
                return;
            }
        }
    }
    emit disconnected();
}

void CdpClient::connectToPage(const QString& wsUrl) {
    m_ws->open(QUrl(wsUrl));
}

void CdpClient::injectScript(const QString& jsCode) {
    if (!isConnected()) return;

    m_cmdId++;
    QJsonObject cmd;
    cmd["id"] = m_cmdId;
    cmd["method"] = "Runtime.evaluate";
    QJsonObject params;
    params["expression"] = jsCode;
    params["awaitPromise"] = false;
    cmd["params"] = params;

    QJsonDocument doc(cmd);
    m_ws->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    emit injectionComplete();
}

bool CdpClient::isConnected() const {
    return m_ws->state() == QAbstractSocket::ConnectedState;
}

void CdpClient::setDisconnectHandler(DisconnectHandler handler) {
    m_disconnectHandler = std::move(handler);
}

void CdpClient::onWsConnected() {
    emit connected();
}

void CdpClient::onWsDisconnected() {
    if (m_disconnectHandler) m_disconnectHandler();
    emit disconnected();
}

void CdpClient::onWsTextMessage(const QString& message) {
    // CDP responses — we don't need to handle them for injection,
    // but could parse for Runtime.evaluate results in the future
    Q_UNUSED(message);
}

void CdpClient::onConnectionTimeout() {
    emit disconnected();
}
```

- [ ] **Step 3: Write tests**

Create `tests/test_cdp_client.cpp`:
```cpp
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "bridge/CdpClient.h"

TEST(CdpClient, initially_not_connected) {
    CdpClient client;
    EXPECT_FALSE(client.isConnected());
}

TEST(CdpClient, connectToDebugger_emits_disconnected_for_invalid_host) {
    CdpClient client;
    QSignalSpy spy(&client, &CdpClient::disconnected);
    client.connectToDebugger("127.0.0.1", 19999); // No CDP server here
    spy.wait(5000);
    EXPECT_GE(spy.count(), 1);
}

TEST(CdpClient, injectScript_noop_when_not_connected) {
    CdpClient client;
    // Should not crash
    client.injectScript("console.log('test');");
}
```

- [ ] **Step 4: Run tests**

```bash
cmake --build build --config Debug
cmake --build build --config Debug --target test
```

- [ ] **Step 5: Commit**

```bash
git add src/bridge/CdpClient.h src/bridge/CdpClient.cpp tests/test_cdp_client.cpp
git commit -m "feat: add CdpClient for Chrome DevTools Protocol injection"
```

---

## Phase 4: Cheat Controller

### Task 8: CheatController

**Files:**
- Create: `src/core/CheatController.h`
- Create: `src/core/CheatController.cpp`
- Create: `tests/test_cheat_controller.cpp`

- [ ] **Step 1: Write CheatController.h**

Create `src/core/CheatController.h`:
```cpp
#pragma once
#include <QObject>
#include <functional>
#include "bridge/Protocol.h"

class CheatController : public QObject {
    Q_OBJECT
public:
    using SendHandler = std::function<void(const QString& json)>;

    explicit CheatController(QObject* parent = nullptr);

    void setSendHandler(SendHandler handler);

    // Read a game state value by path
    void readState(const std::string& path);

    // Write a value to game state
    void writeState(const std::string& path, const nlohmann::json& value);

    // Batch operations
    void batchWrite(const std::vector<std::pair<std::string, nlohmann::json>>& ops);

    // Battle one-shot victory
    void battleVictory();

    // Add item to inventory
    void addItem(int id, const std::string& type, int count);

    // Handle incoming message from game
    void handleMessage(const QString& message);

signals:
    void stateUpdated(const QString& path, const QString& value);
    void operationFailed(const QString& reason);

private:
    SendHandler m_sendHandler;
    void sendCommand(const std::string& json);
};
```

- [ ] **Step 2: Write CheatController.cpp**

Create `src/core/CheatController.cpp`:
```cpp
#include "core/CheatController.h"
#include <QJsonDocument>
#include <QJsonObject>

CheatController::CheatController(QObject* parent)
    : QObject(parent) {}

void CheatController::setSendHandler(SendHandler handler) {
    m_sendHandler = std::move(handler);
}

void CheatController::sendCommand(const std::string& json) {
    if (m_sendHandler) {
        m_sendHandler(QString::fromStdString(json));
    }
}

void CheatController::readState(const std::string& path) {
    Protocol::ReadCommand cmd{path};
    sendCommand(cmd.toJson().dump());
}

void CheatController::writeState(const std::string& path, const nlohmann::json& value) {
    Protocol::WriteCommand cmd{path, value};
    sendCommand(cmd.toJson().dump());
}

void CheatController::batchWrite(const std::vector<std::pair<std::string, nlohmann::json>>& ops) {
    Protocol::BatchCommand batch;
    for (const auto& [path, value] : ops) {
        batch.ops.push_back(Protocol::WriteCommand{path, value}.toJson());
    }
    sendCommand(batch.toJson().dump());
}

void CheatController::battleVictory() {
    Protocol::BattleVictoryCommand cmd;
    sendCommand(cmd.toJson().dump());
}

void CheatController::addItem(int id, const std::string& type, int count) {
    Protocol::AddItemCommand cmd{id, type, count};
    sendCommand(cmd.toJson().dump());
}

void CheatController::handleMessage(const QString& message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    QString type = obj.value("type").toString();

    if (type == "state") {
        QString path = obj.value("path").toString();
        QString val = QString::fromUtf8(QJsonDocument(obj.value("value").toObject()).toJson(QJsonDocument::Compact));
        emit stateUpdated(path, val);
    } else if (type == "error") {
        QString reason = obj.value("reason").toString();
        emit operationFailed(reason);
    }
}
```

- [ ] **Step 3: Write tests**

Create `tests/test_cheat_controller.cpp`:
```cpp
#include <gtest/gtest.h>
#include <QSignalSpy>
#include "core/CheatController.h"

TEST(CheatController, readState_sends_read_command) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.readState("party.gold");

    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "read");
    EXPECT_EQ(j["path"], "party.gold");
}

TEST(CheatController, writeState_sends_write_command) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.writeState("party.gold", 99999);

    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "write");
    EXPECT_EQ(j["path"], "party.gold");
    EXPECT_EQ(j["value"], 99999);
}

TEST(CheatController, battleVictory_sends_correct_type) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.battleVictory();

    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "battle_victory");
}

TEST(CheatController, addItem_sends_item_info) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.addItem(10, "item", 5);

    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "add_item");
    EXPECT_EQ(j["id"], 10);
    EXPECT_EQ(j["count"], 5);
}

TEST(CheatController, handleMessage_state_triggers_signal) {
    CheatController ctrl;
    QSignalSpy spy(&ctrl, &CheatController::stateUpdated);
    ctrl.handleMessage(R"({"type":"state","path":"party.gold","value":999})");
    EXPECT_EQ(spy.count(), 1);
}

TEST(CheatController, handleMessage_error_triggers_signal) {
    CheatController ctrl;
    QSignalSpy spy(&ctrl, &CheatController::operationFailed);
    ctrl.handleMessage(R"({"type":"error","cmd":"read","reason":"bad path"})");
    EXPECT_EQ(spy.count(), 1);
}

TEST(CheatController, batchWrite_sends_multiple_ops) {
    CheatController ctrl;
    std::string captured;
    ctrl.setSendHandler([&](const QString& json) { captured = json.toStdString(); });
    ctrl.batchWrite({{"party.gold", 1000}, {"switches.5", true}});

    auto j = nlohmann::json::parse(captured);
    EXPECT_EQ(j["type"], "batch");
    EXPECT_EQ(j["ops"].size(), 2);
}
```

- [ ] **Step 4: Run tests**

```bash
cmake --build build --config Debug
cmake --build build --config Debug --target test
```

- [ ] **Step 5: Commit**

```bash
git add src/core/CheatController.h src/core/CheatController.cpp tests/test_cheat_controller.cpp
git commit -m "feat: add CheatController with read/write/batch/battle operations"
```

---

## Phase 5: JavaScript Injection Scripts

### Task 9: Hook Scripts — WebSocket Client + State Reader

**Files:**
- Create: `inject/ws-client.js`
- Create: `inject/state-reader.js`
- Create: `tests/test_hook_js.js`

- [ ] **Step 1: Write ws-client.js**

Create `inject/ws-client.js`:
```javascript
// WebSocket client — injected into the game process
(function() {
  if (window.__rtranslator_ws) return; // prevent double-injection
  window.__rtranslator_ws = null;
  window.__rtranslator_callbacks = {};

  function connect(port) {
    var ws = new WebSocket('ws://127.0.0.1:' + port);
    ws.onopen = function() { console.log('[RTranslator] WS connected'); };
    ws.onmessage = function(event) {
      try {
        var msg = JSON.parse(event.data);
        handleMessage(msg);
      } catch(e) { console.error('[RTranslator] Bad message:', e); }
    };
    ws.onclose = function() {
      console.log('[RTranslator] WS disconnected');
      window.__rtranslator_ws = null;
    };
    ws.onerror = function(err) {
      console.error('[RTranslator] WS error:', err);
    };
    window.__rtranslator_ws = ws;
  }

  function send(data) {
    if (window.__rtranslator_ws && window.__rtranslator_ws.readyState === WebSocket.OPEN) {
      window.__rtranslator_ws.send(JSON.stringify(data));
    }
  }

  function on(type, callback) {
    if (!window.__rtranslator_callbacks[type]) {
      window.__rtranslator_callbacks[type] = [];
    }
    window.__rtranslator_callbacks[type].push(callback);
  }

  function handleMessage(msg) {
    var cbs = window.__rtranslator_callbacks[msg.type] || [];
    for (var i = 0; i < cbs.length; i++) { cbs[i](msg); }
  }

  window.__rtranslator_connect = connect;
  window.__rtranslator_send = send;
  window.__rtranslator_on = on;
})();
```

- [ ] **Step 2: Write state-reader.js**

Create `inject/state-reader.js`:
```javascript
// Game state path-based accessor
(function() {
  if (window.__rtranslator_state) return;
  window.__rtranslator_state = {};

  function resolvePath(path) {
    // party.gold -> [$gameParty, 'gold']
    // actors.1.hp -> [$gameActors, 'actor', 1, 'hp']
    // switches.42 -> [$gameSwitches, 'value', 42]
    // variables.7 -> [$gameVariables, 'value', 7]
    // items.5.count -> [$gameParty, 'numItems', $dataItems[5]]
    var parts = path.split('.');
    var segment = parts[0];

    if (segment === 'party') {
      return resolvePartyPath(parts.slice(1));
    } else if (segment === 'actors') {
      return resolveActorPath(parts.slice(1));
    } else if (segment === 'switches') {
      return resolveSwitchPath(parts.slice(1));
    } else if (segment === 'variables') {
      return resolveVariablePath(parts.slice(1));
    } else if (segment === 'items') {
      return resolveItemPath(parts.slice(1));
    }
    return null;
  }

  function resolvePartyPath(parts) {
    if (!window.$gameParty) return null;
    return {
      get: function() {
        if (parts[0] === 'gold') return $gameParty.gold();
        if (parts[0] === 'steps') return $gameParty.steps();
        return null;
      },
      set: function(val) {
        if (parts[0] === 'gold') { $gameParty._gold = val; return true; }
        return false;
      }
    };
  }

  function resolveActorPath(parts) {
    if (!window.$gameActors || parts.length < 2) return null;
    var actorId = parseInt(parts[0], 10);
    var prop = parts[1];
    return {
      get: function() {
        var actor = $gameActors.actor(actorId);
        if (!actor) return null;
        if (prop === 'hp') return actor.hp;
        if (prop === 'mp') return actor.mp;
        if (prop === 'level') return actor._level;
        if (prop === 'exp') return actor.currentExp();
        if (prop === 'name') return actor.name();
        if (prop === 'atk') return actor.atk;
        if (prop === 'def') return actor.def;
        if (prop === 'mat') return actor.mat;
        if (prop === 'mdf') return actor.mdf;
        if (prop === 'agi') return actor.agi;
        if (prop === 'luk') return actor.luk;
        return null;
      },
      set: function(val) {
        var actor = $gameActors.actor(actorId);
        if (!actor) return false;
        if (prop === 'hp') { actor._hp = Math.min(val, actor.mhp); return true; }
        if (prop === 'mp') { actor._mp = Math.min(val, actor.mmp); return true; }
        if (prop === 'level') { actor._level = val; return true; }
        if (prop === 'exp') { actor._exp = {1: val}; return true; }
        return false;
      }
    };
  }

  function resolveSwitchPath(parts) {
    if (!window.$gameSwitches) return null;
    var switchId = parseInt(parts[0], 10);
    return {
      get: function() { return $gameSwitches.value(switchId); },
      set: function(val) { $gameSwitches._data[switchId] = !!val; return true; }
    };
  }

  function resolveVariablePath(parts) {
    if (!window.$gameVariables) return null;
    var varId = parseInt(parts[0], 10);
    return {
      get: function() { return $gameVariables.value(varId); },
      set: function(val) { $gameVariables._data[varId] = val; return true; }
    };
  }

  function resolveItemPath(parts) {
    if (!window.$gameParty || !window.$dataItems) return null;
    var itemId = parseInt(parts[0], 10);
    return {
      get: function() { return $gameParty.numItems($dataItems[itemId]); },
      set: function(val) {
        var current = $gameParty.numItems($dataItems[itemId]);
        var diff = val - current;
        if (diff > 0) $gameParty.gainItem($dataItems[itemId], diff);
        else if (diff < 0) $gameParty.loseItem($dataItems[itemId], -diff);
        return true;
      }
    };
  }

  function readState(path) {
    var accessor = resolvePath(path);
    if (accessor) return accessor.get();
    return null;
  }

  function writeState(path, value) {
    var accessor = resolvePath(path);
    if (accessor) return accessor.set(value);
    return false;
  }

  window.__rtranslator_readState = readState;
  window.__rtranslator_writeState = writeState;
})();
```

- [ ] **Step 3: Create test for JS hook scripts**

Create `tests/test_hook_js.js`:
```javascript
// Node.js unit test for hook scripts
// Run with: node tests/test_hook_js.js

// Mock RPG Maker globals
global.$gameParty = {
  _gold: 500,
  gold: function() { return this._gold; },
  steps: function() { return 42; },
  gainItem: function() {},
  loseItem: function() {},
  numItems: function() { return 3; }
};
global.$gameSwitches = {
  _data: {},
  value: function(id) { return this._data[id] || false; }
};
global.$gameVariables = {
  _data: {},
  value: function(id) { return this._data[id] || 0; }
};
global.$gameActors = {
  actor: function(id) {
    return id === 1 ? { hp: 100, mp: 50, _level: 5, name: function(){return "Hero";},
      atk:20, def:15, mat:10, mdf:10, agi:12, luk:8, mhp:100, mmp:50, currentExp: function(){return 0;} } : null;
  }
};
global.$dataItems = [];

// Load state reader
require('../inject/state-reader.js');

// Test
var errors = [];
function assert(condition, msg) {
  if (!condition) errors.push('FAIL: ' + msg);
}

var g = window.__rtranslator_readState('party.gold');
assert(g === 500, 'party.gold should be 500, got ' + g);

var hp = window.__rtranslator_readState('actors.1.hp');
assert(hp === 100, 'actors.1.hp should be 100, got ' + hp);

window.__rtranslator_writeState('party.gold', 9999);
assert(global.$gameParty._gold === 9999, 'party.gold should be 9999 after write');

window.__rtranslator_writeState('actors.1.hp', 50);
// HP should be clamped by mhp=100, but we set _hp directly so it's 50
assert(global.$gameActors.actor(1).hp === 50, 'actors.1.hp should be 50 after write');

if (errors.length === 0) {
  console.log('All JS hook tests PASSED');
  process.exit(0);
} else {
  console.error(errors.join('\n'));
  process.exit(1);
}
```

- [ ] **Step 4: Run JS tests**

```bash
node tests/test_hook_js.js
```
Expected: `All JS hook tests PASSED`

- [ ] **Step 5: Commit**

```bash
git add inject/ tests/test_hook_js.js
git commit -m "feat: add injected JS hook scripts (ws-client, state-reader)"
```

---

### Task 10: Hook Scripts — Translation Interceptor

**Files:**
- Create: `inject/translation.js`
- Create: `inject/hook.js`

- [ ] **Step 1: Write translation.js**

Create `inject/translation.js`:
```javascript
// Translation interceptor — hooks RPG Maker text rendering
(function() {
  if (window.__rtranslator_trans) return;
  window.__rtranslator_trans = {};

  var translationMap = {};
  var originalTexts = {}; // whitelist set for safety

  function init(map) {
    translationMap = map || {};
  }

  function setWhitelist(whitelist) {
    originalTexts = {};
    for (var i = 0; i < whitelist.length; i++) {
      originalTexts[whitelist[i]] = true;
    }
  }

  function translate(text) {
    if (!text || typeof text !== 'string') return text;
    // Only translate if the text is in the whitelist (pre-extracted from game data)
    // AND we have a translation for it
    var translated = translationMap[text];
    if (translated !== undefined) return translated;
    return text;
  }

  function installHooks() {
    if (!window.Window_Base) {
      // RPG Maker not yet loaded — try again in 100ms
      setTimeout(installHooks, 100);
      return;
    }

    // Hook: Game_Message.prototype.add
    if (window.Game_Message && window.Game_Message.prototype.add) {
      var _add = Game_Message.prototype.add;
      Game_Message.prototype.add = function(text) {
        _add.call(this, translate(text));
      };
    }

    // Hook: Window_Base.prototype.drawText (for UI labels, names)
    // Note: we don't hook drawText directly because it receives processed text
    // Instead we hook at the data level (Game_Message.add, Window_Help.setText, etc.)

    // Hook: Window_Help.prototype.setText
    if (window.Window_Help && window.Window_Help.prototype.setText) {
      var _setText = Window_Help.prototype.setText;
      Window_Help.prototype.setText = function(text) {
        _setText.call(this, translate(text));
      };
    }

    // Hook: Window_Base.prototype.convertEscapeCharacters
    // We handle translations before escape processing at Game_Message level

    console.log('[RTranslator] Translation hooks installed, ' +
      Object.keys(translationMap).length + ' translations loaded');
  }

  window.__rtranslator_initTrans = init;
  window.__rtranslator_setWhitelist = setWhitelist;
  window.__rtranslator_translate = translate;
  window.__rtranslator_installHooks = installHooks;
})();
```

- [ ] **Step 2: Write hook.js (main entry point)**

Create `inject/hook.js`:
```javascript
// RTranslator main hook — combined entry point injected via CDP
(function() {
  var WS_PORT = __RTRANSLATOR_WS_PORT__;
  var TRANSLATION_MAP = __RTRANSLATOR_MAP__;

  function start() {
    // ws-client.js must already be loaded
    window.__rtranslator_connect(WS_PORT);

    window.__rtranslator_on('init', function(msg) {
      if (msg.map) {
        window.__rtranslator_initTrans(msg.map);
      }
      window.__rtranslator_installHooks();
    });

    window.__rtranslator_on('read', function(msg) {
      var val = window.__rtranslator_readState(msg.path);
      window.__rtranslator_send({
        type: 'state',
        path: msg.path,
        value: val
      });
    });

    window.__rtranslator_on('write', function(msg) {
      var ok = window.__rtranslator_writeState(msg.path, msg.value);
      window.__rtranslator_send({
        type: 'ack',
        cmd: 'write',
        success: ok
      });
    });

    window.__rtranslator_on('batch', function(msg) {
      for (var i = 0; i < (msg.ops || []).length; i++) {
        var op = msg.ops[i];
        if (op.type === 'write') {
          window.__rtranslator_writeState(op.path, op.value);
        }
      }
      window.__rtranslator_send({ type: 'ack', cmd: 'batch', success: true });
    });

    window.__rtranslator_on('battle_victory', function() {
      if (window.$gameParty && window.$gameParty.inBattle()) {
        var members = window.$gameTroop ? window.$gameTroop.members() : [];
        for (var i = 0; i < members.length; i++) {
          members[i].setHp(0);
        }
        window.__rtranslator_send({ type: 'ack', cmd: 'battle_victory', success: true });
      } else {
        window.__rtranslator_send({ type: 'error', cmd: 'battle_victory', reason: 'not in battle' });
      }
    });

    window.__rtranslator_on('add_item', function(msg) {
      if (window.$gameParty && window.$dataItems) {
        var data = null;
        if (msg.itemType === 'item') data = window.$dataItems[msg.id];
        else if (msg.itemType === 'weapon') data = window.$dataWeapons[msg.id];
        else if (msg.itemType === 'armor') data = window.$dataArmors[msg.id];
        if (data) {
          window.$gameParty.gainItem(data, msg.count || 1);
          window.__rtranslator_send({ type: 'ack', cmd: 'add_item', success: true });
        }
      }
    });

    window.__rtranslator_on('reload_trans', function(msg) {
      if (msg.map) {
        window.__rtranslator_initTrans(msg.map);
        window.__rtranslator_send({ type: 'ack', cmd: 'reload_trans', success: true });
      }
    });
  }

  // Wait for RPG Maker to be ready
  var tries = 0;
  function waitForReady() {
    tries++;
    if (window.Window_Base || tries > 50) {
      start();
    } else {
      setTimeout(waitForReady, 200);
    }
  }
  waitForReady();
})();
```

- [ ] **Step 3: Commit**

```bash
git add inject/translation.js inject/hook.js
git commit -m "feat: add translation interceptor and main hook script"
```

---

## Phase 6: Game Manager

### Task 11: GameManager — Launch & Lifecycle

**Files:**
- Create: `src/core/GameManager.h`
- Create: `src/core/GameManager.cpp`

- [ ] **Step 1: Write GameManager.h**

Create `src/core/GameManager.h`:
```cpp
#pragma once
#include <QObject>
#include <QProcess>
#include <memory>
#include "bridge/CdpClient.h"
#include "bridge/WebSocketServer.h"
#include "bridge/Protocol.h"

class GameManager : public QObject {
    Q_OBJECT
public:
    explicit GameManager(QObject* parent = nullptr);

    void setGamePath(const QString& path);
    QString gamePath() const;

    void launch();
    void detach();

    bool isRunning() const;
    CdpClient* cdpClient() const;
    WebSocketServer* wsServer() const;
    quint16 cdpPort() const;

signals:
    void gameStarted();
    void gameStopped();
    void injectionSucceeded();
    void injectionFailed(const QString& reason);
    void connectionLost();

private slots:
    void onCdpConnected();
    void onCdpDisconnected();
    void onWsClientConnected();
    void onWsDisconnected();
    void onGameProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    void injectHookScript();
    void injectSupportScript(const QString& name, const QString& script);
    void sendInitMessage();
    QString loadHookScript() const;
    bool findAvailablePort();

    QProcess* m_process;
    CdpClient* m_cdpClient;
    WebSocketServer* m_wsServer;
    QString m_gamePath;
    quint16 m_cdpPort;
    bool m_running;
    bool m_hookInjected;
};
```

- [ ] **Step 2: Write GameManager.cpp**

Create `src/core/GameManager.cpp`:
```cpp
#include "core/GameManager.h"
#include <QDir>
#include <QFile>
#include <QTcpServer>
#include <QMessageBox>

GameManager::GameManager(QObject* parent)
    : QObject(parent)
    , m_process(new QProcess(this))
    , m_cdpClient(new CdpClient(this))
    , m_wsServer(new WebSocketServer(this))
    , m_cdpPort(9222)
    , m_running(false)
    , m_hookInjected(false)
{
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &GameManager::onGameProcessFinished);
    connect(m_cdpClient, &CdpClient::connected, this, &GameManager::onCdpConnected);
    connect(m_cdpClient, &CdpClient::disconnected, this, &GameManager::onCdpDisconnected);
    connect(m_wsServer, &WebSocketServer::clientConnected, this, &GameManager::onWsClientConnected);
    connect(m_wsServer, &WebSocketServer::clientDisconnected, this, &GameManager::onWsDisconnected);
}

void GameManager::setGamePath(const QString& path) { m_gamePath = path; }
QString GameManager::gamePath() const { return m_gamePath; }

bool GameManager::isRunning() const { return m_running; }
CdpClient* GameManager::cdpClient() const { return m_cdpClient; }
WebSocketServer* GameManager::wsServer() const { return m_wsServer; }
quint16 GameManager::cdpPort() const { return m_cdpPort; }

bool GameManager::findAvailablePort() {
    for (int attempt = 0; attempt < 5; attempt++) {
        QTcpServer test;
        if (test.listen(QHostAddress::LocalHost, m_cdpPort)) {
            test.close();
            return true;
        }
        m_cdpPort++;
    }
    return false;
}

void GameManager::launch() {
    if (m_gamePath.isEmpty()) {
        emit injectionFailed("No game path set");
        return;
    }

    // Start WebSocket server first
    if (!m_wsServer->start(0)) {
        emit injectionFailed("Failed to start WebSocket server");
        return;
    }

    // Find available CDP port
    if (!findAvailablePort()) {
        emit injectionFailed("No available CDP port (tried 9222-9226)");
        return;
    }

    // Launch game with --remote-debugging-port
    QStringList args;
    args << QString("--remote-debugging-port=%1").arg(m_cdpPort);

    QFileInfo exeInfo(m_gamePath);
    m_process->setWorkingDirectory(exeInfo.absolutePath());
    m_process->start(m_gamePath, args);

    if (!m_process->waitForStarted(5000)) {
        emit injectionFailed("Failed to start game process");
        return;
    }

    m_running = true;
    emit gameStarted();

    // Connect CDP after a brief delay to let NW.js initialize
    QTimer::singleShot(1000, this, [this]() {
        m_cdpClient->connectToDebugger("127.0.0.1", m_cdpPort);
    });
}

void GameManager::detach() {
    if (m_process && m_running) {
        m_process->closeWriteChannel();
        // Process continues running independently
        m_running = false;
        m_hookInjected = false;
        emit gameStopped();
    }
}

void GameManager::onCdpConnected() {
    // Inject the hook scripts
    injectHookScript();
}

void GameManager::onCdpDisconnected() {
    if (!m_hookInjected && m_running) {
        emit injectionFailed("CDP connection failed or lost");
    }
}

void GameManager::onWsClientConnected() {
    m_hookInjected = true;
    sendInitMessage();
    emit injectionSucceeded();
}

void GameManager::onWsDisconnected() {
    emit connectionLost();
}

void GameManager::injectHookScript() {
    // Inject ws-client.js first
    QString wsClient = loadTextFile("inject/ws-client.js");
    injectSupportScript("ws-client.js", wsClient);

    // Inject state-reader.js
    QString stateReader = loadTextFile("inject/state-reader.js");
    injectSupportScript("state-reader.js", stateReader);

    // Inject translation.js
    QString trans = loadTextFile("inject/translation.js");
    injectSupportScript("translation.js", trans);

    // Inject hook.js with placeholders replaced
    QString hook = loadTextFile("inject/hook.js");
    hook.replace("__RTRANSLATOR_WS_PORT__", QString::number(m_wsServer->port()));
    m_cdpClient->injectScript(hook);
}

void GameManager::injectSupportScript(const QString&, const QString& script) {
    m_cdpClient->injectScript(script);
}

void GameManager::sendInitMessage() {
    Protocol::InitCommand init;
    init.translationMap = {}; // Will be set from TranslationEngine
    QString json = QString::fromStdString(init.toJson().dump());
    m_wsServer->sendMessage(json);
}

void GameManager::onGameProcessFinished(int exitCode, QProcess::ExitStatus status) {
    Q_UNUSED(exitCode); Q_UNUSED(status);
    m_running = false;
    m_hookInjected = false;
    emit gameStopped();
}

QString GameManager::loadTextFile(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return QString::fromUtf8(file.readAll());
}
```

- [ ] **Step 3: Commit**

```bash
git add src/core/GameManager.h src/core/GameManager.cpp
git commit -m "feat: add GameManager for game process lifecycle and hook injection"
```

---

## Phase 7: QML UI

### Task 12: MainWindow C++ Bridge + main.qml Update

**Files:**
- Create: `src/app/MainWindow.h`
- Create: `src/app/MainWindow.cpp`
- Modify: `src/main.cpp`
- Modify: `qml/main.qml`
- Modify: `qml/qml.qrc`

- [ ] **Step 1: Write MainWindow.h**

Create `src/app/MainWindow.h`:
```cpp
#pragma once
#include <QObject>
#include <QQmlApplicationEngine>
#include <memory>
#include "core/GameManager.h"
#include "core/TranslationEngine.h"
#include "core/CheatController.h"
#include "adapter/RpgMVAdapter.h"

class MainWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString gamePath READ gamePath WRITE setGamePath NOTIFY gamePathChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString translationStats READ translationStats NOTIFY translationStatsChanged)

public:
    explicit MainWindow(QQmlApplicationEngine* engine, QObject* parent = nullptr);

    QString gamePath() const;
    void setGamePath(const QString& path);

    bool isConnected() const;
    QString statusText() const;
    QString translationStats() const;

public slots:
    void launchGame();
    void detachGame();
    void loadGameDir(const QString& dir);
    void loadTranslationFile(const QString& filePath);
    void extractText();
    void readGameState(const QString& path);
    void writeGameState(const QString& path, const QVariant& value);
    void triggerBattleVictory();
    void addItemToInventory(int id, const QString& type, int count);

signals:
    void gamePathChanged();
    void connectionChanged();
    void statusTextChanged();
    void translationStatsChanged();
    void gameStarted();
    void gameStopped();
    void extractionComplete(const QVariantList& entries);
    void translationLoaded(int count);

private slots:
    void onInjectionSucceeded();
    void onInjectionFailed(const QString& reason);

private:
    QQmlApplicationEngine* m_engine;
    std::unique_ptr<TranslationEngine> m_transEngine;
    std::unique_ptr<GameManager> m_gameManager;
    std::unique_ptr<CheatController> m_cheatController;
    QString m_gamePath;
    QString m_statusText;
    QString m_transStats;
    bool m_connected;
};
```

- [ ] **Step 2: Write MainWindow.cpp**

Create `src/app/MainWindow.cpp`:
```cpp
#include "app/MainWindow.h"
#include <QDir>
#include <QFileInfo>
#include <QQmlContext>

MainWindow::MainWindow(QQmlApplicationEngine* engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
    , m_transEngine(std::make_unique<TranslationEngine>(std::make_unique<RpgMVAdapter>()))
    , m_gameManager(std::make_unique<GameManager>(this))
    , m_cheatController(std::make_unique<CheatController>(this))
    , m_connected(false)
{
    connect(m_gameManager.get(), &GameManager::gameStarted, this, &MainWindow::gameStarted);
    connect(m_gameManager.get(), &GameManager::gameStopped, this, &MainWindow::gameStopped);
    connect(m_gameManager.get(), &GameManager::injectionSucceeded, this, &MainWindow::onInjectionSucceeded);
    connect(m_gameManager.get(), &GameManager::injectionFailed, this, &MainWindow::onInjectionFailed);
    connect(m_gameManager.get(), &GameManager::connectionLost, this, [this]() {
        m_connected = false;
        m_statusText = "Connection lost";
        emit statusTextChanged();
        emit connectionChanged();
    });

    // Wire CheatController to GameManager
    m_cheatController->setSendHandler([this](const QString& json) {
        if (m_gameManager->wsServer()) {
            m_gameManager->wsServer()->sendMessage(json);
        }
    });

    // Wire WebSocket messages to CheatController
    if (m_gameManager->wsServer()) {
        m_gameManager->wsServer()->setMessageHandler([this](const QString& msg) {
            m_cheatController->handleMessage(msg);
        });
    }
}

QString MainWindow::gamePath() const { return m_gamePath; }
void MainWindow::setGamePath(const QString& path) {
    m_gamePath = path;
    emit gamePathChanged();
}

bool MainWindow::isConnected() const { return m_connected; }
QString MainWindow::statusText() const { return m_statusText; }
QString MainWindow::translationStats() const { return m_transStats; }

void MainWindow::launchGame() {
    m_gameManager->setGamePath(m_gamePath);
    m_gameManager->launch();
}

void MainWindow::detachGame() {
    m_gameManager->detach();
}

void MainWindow::loadGameDir(const QString& dir) {
    if (!m_transEngine->detectGame(dir.toStdString())) {
        m_statusText = "Not a valid RPG Maker MV/MZ game";
        emit statusTextChanged();
        return;
    }
    // Find exe
    QDir gameDir(dir);
    QStringList exes = gameDir.entryList({"*.exe"}, QDir::Files);
    for (const auto& exe : exes) {
        if (exe.compare("Game.exe", Qt::CaseInsensitive) == 0 ||
            exe.contains("nw", Qt::CaseInsensitive)) {
            m_gamePath = gameDir.absoluteFilePath(exe);
            emit gamePathChanged();
            m_statusText = "Game detected: " + dir;
            emit statusTextChanged();
            return;
        }
    }
    m_statusText = "No game executable found in " + dir;
    emit statusTextChanged();
}

void MainWindow::loadTranslationFile(const QString& filePath) {
    if (m_transEngine->loadTranslation(filePath.toStdString())) {
        int count = static_cast<int>(m_transEngine->translationMap().size());
        m_transStats = QString("%1 translations loaded").arg(count);
        emit translationStatsChanged();
        emit translationLoaded(count);

        // Push translation map to game if connected
        if (m_gameManager->wsServer()) {
            Protocol::ReloadTransCommand cmd;
            cmd.translationMap = m_transEngine->translationMap();
            QString json = QString::fromStdString(cmd.toJson().dump());
            m_gameManager->wsServer()->sendMessage(json);
        }
    } else {
        m_statusText = "Failed to load translation file";
        emit statusTextChanged();
    }
}

void MainWindow::extractText() {
    if (!m_gamePath.isEmpty()) {
        QFileInfo info(m_gamePath);
        auto entries = m_transEngine->extractFromGame(info.absolutePath().toStdString());
        QVariantList list;
        for (const auto& e : entries) {
            QVariantMap map;
            map["id"] = QString::fromStdString(e.id);
            map["source"] = QString::fromStdString(e.source);
            map["context"] = QString::fromStdString(e.context);
            map["original"] = QString::fromStdString(e.original);
            map["translation"] = QString::fromStdString(e.translation);
            list.append(map);
        }
        m_transStats = QString("%1 strings extracted").arg(list.size());
        emit translationStatsChanged();
        emit extractionComplete(list);
    }
}

void MainWindow::readGameState(const QString& path) {
    m_cheatController->readState(path.toStdString());
}

void MainWindow::writeGameState(const QString& path, const QVariant& value) {
    // Convert QVariant to nlohmann::json
    nlohmann::json j;
    if (value.type() == QVariant::Bool) {
        j = value.toBool();
    } else if (value.type() == QVariant::Int || value.type() == QVariant::UInt) {
        j = value.toInt();
    } else if (value.type() == QVariant::Double) {
        j = value.toDouble();
    } else {
        j = value.toString().toStdString();
    }
    m_cheatController->writeState(path.toStdString(), j);
}

void MainWindow::triggerBattleVictory() {
    m_cheatController->battleVictory();
}

void MainWindow::addItemToInventory(int id, const QString& type, int count) {
    m_cheatController->addItem(id, type.toStdString(), count);
}

void MainWindow::onInjectionSucceeded() {
    m_connected = true;
    m_statusText = "Connected";
    emit statusTextChanged();
    emit connectionChanged();
}

void MainWindow::onInjectionFailed(const QString& reason) {
    m_connected = false;
    m_statusText = "Failed: " + reason;
    emit statusTextChanged();
    emit connectionChanged();
}
```

- [ ] **Step 3: Update main.cpp to register MainWindow**

Modify `src/main.cpp`:
```cpp
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "app/MainWindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("RTranslator");
    app.setApplicationName("RTranslator");

    QQmlApplicationEngine engine;
    MainWindow mainWindow(&engine);

    engine.rootContext()->setContextProperty("mainWindow", &mainWindow);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);
    return app.exec();
}
```

- [ ] **Step 4: Update main.qml**

Write `qml/main.qml`:
```qml
import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: root
    width: 1280
    height: 800
    visible: true
    title: "RTranslator"
    color: "#1e1e2e"
    minimumWidth: 900
    minimumHeight: 600

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        TopBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            TranslationPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.4
            }

            Rectangle { width: 1; Layout.fillHeight: true; color: "#313244" }

            ModificationPanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.3
            }

            Rectangle { width: 1; Layout.fillHeight: true; color: "#313244" }

            GameStatePanel {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.preferredWidth: root.width * 0.3
            }
        }

        StatusBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 28
        }
    }
}
```

- [ ] **Step 5: Update qml.qrc**

```xml
<RCC>
    <qresource prefix="/qml">
        <file>main.qml</file>
        <file>TopBar.qml</file>
        <file>TranslationPanel.qml</file>
        <file>ModificationPanel.qml</file>
        <file>GameStatePanel.qml</file>
        <file>StatusBar.qml</file>
    </qresource>
</RCC>
```

- [ ] **Step 6: Commit**

```bash
git add src/app/MainWindow.h src/app/MainWindow.cpp src/main.cpp qml/main.qml qml/qml.qrc
git commit -m "feat: add MainWindow C++ bridge and update QML root layout"
```

---

### Task 13: TopBar, StatusBar, ModificationPanel, GameStatePanel QML

**Files:**
- Create: `qml/TopBar.qml`
- Create: `qml/StatusBar.qml`
- Create: `qml/ModificationPanel.qml`
- Create: `qml/GameStatePanel.qml`

- [ ] **Step 1: Write TopBar.qml**

Create `qml/TopBar.qml`:
```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#181825"
    border.color: "#313244"

    RowLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 8

        Text {
            text: "RTranslator"
            color: "#cba6f7"
            font.pixelSize: 16
            font.bold: true
        }

        Rectangle { width: 1; height: 28; color: "#313244" }

        TextField {
            id: pathField
            Layout.fillWidth: true
            placeholderText: "Game path..."
            text: mainWindow.gamePath
            color: "#cdd6f4"
            background: Rectangle { color: "#313244"; radius: 4 }
            onTextChanged: mainWindow.setGamePath(text)
        }

        Button {
            text: "Browse..."
            onClicked: {
                // Will use Qt Quick Dialogs FolderDialog in final version
            }
        }

        Button {
            text: "Launch"
            enabled: !mainWindow.isConnected && mainWindow.gamePath !== ""
            onClicked: mainWindow.launchGame()
            background: Rectangle {
                color: parent.enabled ? "#a6e3a1" : "#45475a"
                radius: 4
            }
        }

        Button {
            text: "Detach"
            enabled: mainWindow.isConnected
            onClicked: mainWindow.detachGame()
            background: Rectangle {
                color: parent.enabled ? "#f38ba8" : "#45475a"
                radius: 4
            }
        }
    }
}
```

- [ ] **Step 2: Write StatusBar.qml**

Create `qml/StatusBar.qml`:
```qml
import QtQuick
import QtQuick.Layouts

Rectangle {
    color: "#181825"
    border.color: "#313244"

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        spacing: 12

        Rectangle {
            width: 10; height: 10; radius: 5
            color: mainWindow.isConnected ? "#a6e3a1" : "#f38ba8"
        }

        Text {
            text: mainWindow.statusText
            color: "#a6adc8"
            font.pixelSize: 11
        }

        Rectangle { Layout.fillWidth: true }

        Text {
            text: mainWindow.translationStats
            color: "#a6adc8"
            font.pixelSize: 11
        }
    }
}
```

- [ ] **Step 3: Write ModificationPanel.qml**

Create `qml/ModificationPanel.qml`:
```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#1e1e2e"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Text {
            text: "Modification"
            color: "#cba6f7"
            font.pixelSize: 14
            font.bold: true
        }

        // Gold editor
        RowLayout {
            Text { text: "Gold:"; color: "#cdd6f4"; Layout.preferredWidth: 60 }
            TextField {
                id: goldField
                Layout.fillWidth: true
                text: "0"
                color: "#cdd6f4"
                background: Rectangle { color: "#313244"; radius: 4 }
            }
            Button {
                text: "Set"
                onClicked: mainWindow.writeGameState("party.gold", parseInt(goldField.text))
            }
            Button {
                text: "Read"
                onClicked: mainWindow.readGameState("party.gold")
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

        // HP/MP for actor 1
        Text { text: "Actor 1"; color: "#89b4fa"; font.pixelSize: 12; font.bold: true }
        RowLayout {
            Text { text: "HP:"; color: "#cdd6f4"; Layout.preferredWidth: 60 }
            TextField {
                id: hpField
                Layout.fillWidth: true
                text: "0"
                color: "#cdd6f4"
                background: Rectangle { color: "#313244"; radius: 4 }
            }
            Button {
                text: "Set"
                onClicked: mainWindow.writeGameState("actors.1.hp", parseInt(hpField.text))
            }
        }

        RowLayout {
            Text { text: "MP:"; color: "#cdd6f4"; Layout.preferredWidth: 60 }
            TextField {
                id: mpField
                Layout.fillWidth: true
                text: "0"
                color: "#cdd6f4"
                background: Rectangle { color: "#313244"; radius: 4 }
            }
            Button {
                text: "Set"
                onClicked: mainWindow.writeGameState("actors.1.mp", parseInt(mpField.text))
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

        // Switch toggle
        RowLayout {
            Text { text: "Switch:"; color: "#cdd6f4"; Layout.preferredWidth: 60 }
            TextField {
                id: switchField
                Layout.fillWidth: true
                text: "1"
                color: "#cdd6f4"
                background: Rectangle { color: "#313244"; radius: 4 }
            }
            Button {
                text: "ON"
                onClicked: mainWindow.writeGameState("switches." + switchField.text, true)
            }
            Button {
                text: "OFF"
                onClicked: mainWindow.writeGameState("switches." + switchField.text, false)
            }
        }

        // Variable editor
        RowLayout {
            Text { text: "Variable:"; color: "#cdd6f4"; Layout.preferredWidth: 60 }
            TextField {
                id: varIdField
                Layout.fillWidth: true
                text: "1"
                color: "#cdd6f4"
                background: Rectangle { color: "#313244"; radius: 4 }
            }
            TextField {
                id: varValField
                Layout.fillWidth: true
                text: "0"
                color: "#cdd6f4"
                background: Rectangle { color: "#313244"; radius: 4 }
            }
            Button {
                text: "Set"
                onClicked: mainWindow.writeGameState("variables." + varIdField.text, parseInt(varValField.text))
            }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

        // Battle victory
        Button {
            Layout.fillWidth: true
            text: "One-Click Victory"
            onClicked: mainWindow.triggerBattleVictory()
            background: Rectangle { color: "#f9e2af"; radius: 4 }
            contentItem: Text {
                text: "One-Click Victory"
                color: "#1e1e2e"
                horizontalAlignment: Text.AlignHCenter
                font.bold: true
            }
        }

        Rectangle { Layout.fillHeight: true }
    }
}
```

- [ ] **Step 4: Write GameStatePanel.qml**

Create `qml/GameStatePanel.qml`:
```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#1e1e2e"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        Text {
            text: "Game State"
            color: "#a6e3a1"
            font.pixelSize: 14
            font.bold: true
        }

        Label {
            text: "Live state display will appear here when connected"
            color: "#6c7086"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            visible: !mainWindow.isConnected
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: mainWindow.isConnected

            ColumnLayout {
                spacing: 6
                // State data will be populated via signal handlers in full implementation
                Repeater {
                    model: ["Party", "Switches", "Variables"]
                    delegate: Rectangle {
                        Layout.fillWidth: true
                        height: 30
                        color: "#313244"
                        radius: 4
                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            color: "#cdd6f4"
                            font.pixelSize: 12
                        }
                    }
                }
            }
        }
    }
}
```

- [ ] **Step 5: Build to verify QML compiles**

```bash
cmake --build build --config Debug
```

- [ ] **Step 6: Commit**

```bash
git add qml/TopBar.qml qml/StatusBar.qml qml/ModificationPanel.qml qml/GameStatePanel.qml
git commit -m "feat: add TopBar, StatusBar, ModificationPanel, GameStatePanel QML"
```

---

### Task 14: TranslationPanel QML

**Files:**
- Create: `qml/TranslationPanel.qml`

- [ ] **Step 1: Write TranslationPanel.qml**

Create `qml/TranslationPanel.qml`:
```qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    color: "#1e1e2e"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        Text {
            text: "Translation"
            color: "#89b4fa"
            font.pixelSize: 14
            font.bold: true
        }

        RowLayout {
            Button {
                text: "Extract Text"
                onClicked: mainWindow.extractText()
            }
            Button {
                text: "Load Translation JSON"
                onClicked: {
                    // File dialog in final version
                }
            }
            Button {
                text: "Export JSON"
                onClicked: {
                    // Export function
                }
            }
        }

        Text {
            text: mainWindow.translationStats
            color: "#a6adc8"
            font.pixelSize: 11
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#313244" }

        // Search bar
        TextField {
            id: searchField
            Layout.fillWidth: true
            placeholderText: "Search strings..."
            color: "#cdd6f4"
            background: Rectangle { color: "#313244"; radius: 4 }
        }

        // Entry list
        ListView {
            id: entryList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: ListModel { id: entryModel }

            delegate: Rectangle {
                width: entryList.width
                height: 40
                color: index % 2 === 0 ? "#1e1e2e" : "#181825"
                border.color: "#313244"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 8

                    Text {
                        Layout.preferredWidth: 100
                        text: model.source || ""
                        color: "#6c7086"
                        font.pixelSize: 10
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: model.original || ""
                        color: "#cdd6f4"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.preferredWidth: 120
                        text: model.translation || ""
                        color: "#a6e3a1"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        // Select for editing
                    }
                }
            }
        }
    }

    Connections {
        target: mainWindow
        function onExtractionComplete(entries) {
            entryModel.clear()
            for (var i = 0; i < entries.length; i++) {
                entryModel.append(entries[i])
            }
        }
    }
}
```

- [ ] **Step 2: Build to verify and commit**

```bash
cmake --build build --config Debug
git add qml/TranslationPanel.qml
git commit -m "feat: add TranslationPanel QML with list and extraction"
```

---

## Phase 8: Integration & Polish

### Task 15: Wire Translation Engine into Game Init

**Files:**
- Modify: `src/core/GameManager.cpp`
- Modify: `src/app/MainWindow.cpp`

- [ ] **Step 1: Add translation map parameter to GameManager**

Modify `src/core/GameManager.h` — add public method:
```cpp
void setTranslationMap(const std::unordered_map<std::string, std::string>& map);
```

Add private member:
```cpp
std::unordered_map<std::string, std::string> m_translationMap;
```

Modify `GameManager.cpp`:
```cpp
void GameManager::setTranslationMap(const std::unordered_map<std::string, std::string>& map) {
    m_translationMap = map;
}

// Update sendInitMessage:
void GameManager::sendInitMessage() {
    Protocol::InitCommand init;
    init.translationMap = m_translationMap;
    QString json = QString::fromStdString(init.toJson().dump());
    m_wsServer->sendMessage(json);
}
```

- [ ] **Step 2: Update MainWindow to pass translation map to GameManager**

Modify `MainWindow.cpp` — in `loadTranslationFile`, after loading:
```cpp
m_gameManager->setTranslationMap(m_transEngine->translationMap());
```

- [ ] **Step 3: Build and commit**

```bash
cmake --build build --config Debug
git add -u
git commit -m "feat: wire translation map into game initialization"
```

---

### Task 16: End-to-End Integration Test

**Files:**
- Create: `tests/test_integration.cpp`

- [ ] **Step 1: Write integration test**

Create `tests/test_integration.cpp`:
```cpp
#include <gtest/gtest.h>
#include <QCoreApplication>
#include "core/TranslationEngine.h"
#include "adapter/RpgMVAdapter.h"
#include "bridge/Protocol.h"
#include "bridge/WebSocketServer.h"
#include "bridge/CdpClient.h"
#include "core/CheatController.h"

// Integration: TranslationEngine → Protocol → message round-trip
TEST(Integration, translationMap_to_protocol_roundtrip) {
    auto engine = std::make_unique<TranslationEngine>(std::make_unique<RpgMVAdapter>());
    ASSERT_TRUE(engine->loadTranslation("tests/test_data/sample_trans.json"));

    Protocol::InitCommand init;
    init.translationMap = engine->translationMap();
    auto json = init.toJson();
    EXPECT_EQ(json["map"]["hello"], "nihao");
    EXPECT_EQ(json["map"]["world"], "shijie");
}

// Integration: CheatController → Protocol → parse response
TEST(Integration, cheat_read_write_roundtrip) {
    CheatController ctrl;
    std::string lastSent;
    ctrl.setSendHandler([&](const QString& json) { lastSent = json.toStdString(); });

    ctrl.writeState("party.gold", 99999);
    auto j = nlohmann::json::parse(lastSent);
    EXPECT_EQ(j["type"], "write");
    EXPECT_EQ(j["value"], 99999);

    // Simulate response from game
    std::string stateMsg = R"({"type":"state","path":"party.gold","value":99999})";
    ctrl.handleMessage(QString::fromStdString(stateMsg));
}

// Integration: WebSocket server accepts connection (smoke test)
TEST(Integration, websocket_server_starts_and_stops) {
    WebSocketServer server;
    ASSERT_TRUE(server.start(0));
    EXPECT_GT(server.port(), 0);
    EXPECT_TRUE(server.isListening());
    server.stop();
    EXPECT_FALSE(server.isListening());
}
```

- [ ] **Step 2: Run all tests**

```bash
cmake --build build --config Debug
cmake --build build --config Debug --target test
```

- [ ] **Step 3: Commit**

```bash
git add tests/test_integration.cpp
git commit -m "test: add integration tests for Protocol, CheatController, WebSocket"
```

---

## Summary

| Phase | Tasks | Key Deliverables |
|-------|-------|-----------------|
| 1: Foundation | 1-3 | CMake, Protocol types, JsonHelper |
| 2: Game Data | 4-5 | IGameAdapter, RpgMVAdapter, TranslationEngine |
| 3: IPC | 6-7 | WebSocketServer, CdpClient |
| 4: Cheats | 8 | CheatController |
| 5: JS Scripts | 9-10 | ws-client, state-reader, translation, hook |
| 6: Game Manager | 11 | GameManager lifecycle |
| 7: UI | 12-14 | MainWindow bridge, all QML panels |
| 8: Integration | 15-16 | Wiring, end-to-end tests |

**Total: 16 tasks, ~96 steps**
