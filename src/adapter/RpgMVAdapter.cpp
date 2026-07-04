#include "adapter/RpgMVAdapter.h"
#include "util/JsonHelper.h"
#include <QDir>
#include <QFileInfo>
#include <cstdio>

bool RpgMVAdapter::detect(const QString& gameDir) {
    QDir dir(gameDir);
    return dir.exists("www/data/System.json") || dir.exists("data/System.json");
}

std::vector<TranslationEntry> RpgMVAdapter::extractText(const QString& gameDir) {
    std::vector<TranslationEntry> entries;
    // Try www/data first, then data as fallback
    QString dataPath = gameDir + "/www/data";
    if (!QFileInfo::exists(dataPath + "/System.json")) {
        dataPath = gameDir + "/data";
    }

    struct FileSpec { QString name; QString prefix; std::vector<std::string> fields; };
    std::vector<FileSpec> specs = {
        {"Actors.json",   "Actor",   {"name", "nickname", "profile", "note"}},
        {"Classes.json",  "Class",   {"name", "note"}},
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
        QString filePath = dataPath + "/" + spec.name;
        if (!QFileInfo::exists(filePath)) continue;
        extractFromFile(filePath, spec.name, spec.prefix, spec.fields, entries);
    }

    QString systemPath = dataPath + "/System.json";
    if (QFileInfo::exists(systemPath)) {
        extractFromSystemFile(systemPath, "System.json", entries);
    }

    QString commonPath = dataPath + "/CommonEvents.json";
    if (QFileInfo::exists(commonPath)) {
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
                        "CommonEvent #" + std::to_string(idx), idx, 0, entries);
                }
                idx++;
            }
        }
    }

    // Use MapInfos.json to find all valid map IDs (works for both MV and MZ)
    QString mapInfosPath = dataPath + "/MapInfos.json";
    auto mapInfosOpt = JsonHelper::loadJsonFile(mapInfosPath);
    int mapCount = 0;
    if (mapInfosOpt.has_value() && mapInfosOpt->is_object()) {
        for (auto& [idStr, info] : mapInfosOpt->items()) {
            int mapId = std::stoi(idStr);
            char mvBuf[64], mzBuf[64];
            snprintf(mvBuf, sizeof(mvBuf), "/Map%03d.json", mapId);
            snprintf(mzBuf, sizeof(mzBuf), "/MapData_%03d.json", mapId);
            QString mvPath = dataPath + QString::fromUtf8(mvBuf);
            QString mzPath = dataPath + QString::fromUtf8(mzBuf);
            // Also try MZ bare-number format: MapData_1.json, MapData_2.json, etc.
            char mzBareBuf[64];
            snprintf(mzBareBuf, sizeof(mzBareBuf), "/MapData_%d.json", mapId);
            QString mzBarePath = dataPath + QString::fromUtf8(mzBareBuf);

            QString actualPath;
            if (QFileInfo::exists(mvPath)) { actualPath = mvPath; }
            else if (QFileInfo::exists(mzPath)) { actualPath = mzPath; }
            else if (QFileInfo::exists(mzBarePath)) { actualPath = mzBarePath; }

            if (!actualPath.isEmpty()) {
                extractFromMapFile(actualPath, QString::fromUtf8("Map%1.json").arg(mapId, 3, 10, QChar('0')), entries);
                mapCount++;
            }
        }
    }
    // Fallback: brute-force scan 1..999 if MapInfos.json wasn't found
    if (mapCount == 0) {
        for (int i = 1; i <= 999; i++) {
            char mvBuf[64];
            snprintf(mvBuf, sizeof(mvBuf), "/Map%03d.json", i);
            QString mapPath = dataPath + QString::fromUtf8(mvBuf);
            if (QFileInfo::exists(mapPath)) {
                extractFromMapFile(mapPath, QString::fromUtf8("Map%1.json").arg(i, 3, 10, QChar('0')), entries);
            }
        }
    }

    return entries;
}

void RpgMVAdapter::extractFromFile(const QString& filePath, const QString& source,
                                    const QString& contextPrefix,
                                    const std::vector<std::string>& fields,
                                    std::vector<TranslationEntry>& out) {
    auto opt = JsonHelper::loadJsonFile(filePath);
    if (!opt.has_value() || !opt->is_array()) return;

    int idx = 0;
    std::string sourceStr = source.toStdString();
    for (const auto& item : *opt) {
        if (item.is_null()) { idx++; continue; }
        for (const auto& field : fields) {
            auto val = JsonHelper::getString(item, field);
            if (val.has_value() && !val->empty()) {
                out.push_back({
                    sourceStr + "/" + std::to_string(idx) + "/" + field,
                    sourceStr,
                    contextPrefix.toStdString() + " #" + std::to_string(idx) + " - " + field,
                    field,
                    *val,
                    ""
                });
            }
        }
        idx++;
    }
}

void RpgMVAdapter::extractFromMapFile(const QString& filePath, const QString& source,
                                       std::vector<TranslationEntry>& out) {
    auto opt = JsonHelper::loadJsonFile(filePath);
    if (!opt.has_value()) return;

    std::string sourceStr = source.toStdString();

    if (opt->contains("displayName")) {
        auto name = JsonHelper::getString(*opt, "displayName");
        if (name.has_value() && !name->empty()) {
            out.push_back({sourceStr + "/displayName", sourceStr, "Map Display Name", "displayName", *name, ""});
        }
    }

    if (opt->contains("events") && (*opt)["events"].is_array()) {
        for (const auto& event : (*opt)["events"]) {
            if (event.is_null()) continue;
            std::string eventName = event.value("name", "");
            int eventId = event.value("id", 0);

            if (!eventName.empty()) {
                out.push_back({sourceStr + "/event" + std::to_string(eventId) + "/name",
                    sourceStr, "Event #" + std::to_string(eventId) + " Name", "name", eventName, ""});
            }

            if (event.contains("pages")) {
                for (const auto& page : event["pages"]) {
                    if (page.is_null() || !page.contains("list")) continue;
                    std::string ctx = "Event #" + std::to_string(eventId) + " Page " + std::to_string(page.value("pageIndex", 0));
                    extractEventParameters(page["list"], sourceStr, ctx, eventId, page.value("pageIndex", 0), out);
                }
            }
        }
    }
}

void RpgMVAdapter::extractEventParameters(const nlohmann::json& list, const std::string& source,
                                           const std::string& context, int eventId, int pageIdx,
                                           std::vector<TranslationEntry>& out) {
    if (!list.is_array()) return;
    int cmdIdx = 0;
    for (const auto& cmd : list) {
        if (!cmd.contains("code") || !cmd.contains("parameters")) {
            cmdIdx++; continue;
        }
        int code = cmd["code"].get<int>();
        const auto& params = cmd["parameters"];

        if (code == 101 || code == 401) {
            if (params.is_array() && params.size() > 0 && params[0].is_string()) {
                std::string text = params[0].get<std::string>();
                if (!text.empty()) {
                    out.push_back({source + "/ev" + std::to_string(eventId) + "/p" + std::to_string(pageIdx) + "/cmd" + std::to_string(cmdIdx),
                        source, context + " - Message", "message", text, ""});
                }
            }
        }
        else if (code == 102) {
            if (params.is_array()) {
                for (size_t pi = 0; pi < params.size(); pi++) {
                    if (params[pi].is_string()) {
                        std::string choice = params[pi].get<std::string>();
                        if (!choice.empty()) {
                            out.push_back({source + "/ev" + std::to_string(eventId) + "/p" + std::to_string(pageIdx) + "/cmd" + std::to_string(cmdIdx) + "/choice" + std::to_string(pi),
                                source, context + " - Choice #" + std::to_string(pi), "choice", choice, ""});
                        }
                    }
                }
            }
        }
        cmdIdx++;
    }
}

void RpgMVAdapter::extractFromSystemFile(const QString& filePath, const QString& source,
                                          std::vector<TranslationEntry>& out) {
    auto opt = JsonHelper::loadJsonFile(filePath);
    if (!opt.has_value()) return;

    nlohmann::json& sys = *opt;
    std::string sourceStr = source.toStdString();

    auto pushSysField = [&](const std::string& field) {
        auto val = JsonHelper::getString(sys, field);
        if (val.has_value() && !val->empty()) {
            out.push_back({sourceStr + "/" + field, sourceStr, "System - " + field, field, *val, ""});
        }
    };
    pushSysField("gameTitle");
    pushSysField("currencyUnit");

    if (sys.contains("elements") && sys["elements"].is_array()) {
        int i = 0;
        for (const auto& elem : sys["elements"]) {
            if (elem.is_string()) {
                std::string s = elem.get<std::string>();
                if (!s.empty()) {
                    out.push_back({sourceStr + "/elements/" + std::to_string(i),
                        sourceStr, "System - Element #" + std::to_string(i), "element", s, ""});
                }
            }
            i++;
        }
    }

    if (sys.contains("terms") && sys["terms"].is_object()) {
        for (auto& [termKey, termVal] : sys["terms"].items()) {
            if (termVal.is_string()) {
                std::string s = termVal.get<std::string>();
                if (!s.empty()) {
                    out.push_back({sourceStr + "/terms/" + termKey,
                        sourceStr, "System - Term: " + termKey, "term", s, ""});
                }
            } else if (termVal.is_array()) {
                for (size_t i = 0; i < termVal.size(); i++) {
                    if (termVal[i].is_string()) {
                        std::string s = termVal[i].get<std::string>();
                        if (!s.empty()) {
                            out.push_back({sourceStr + "/terms/" + termKey + "/" + std::to_string(i),
                                sourceStr, "System - Term: " + termKey + "[" + std::to_string(i) + "]", "term", s, ""});
                        }
                    }
                }
            }
        }
    }
}
