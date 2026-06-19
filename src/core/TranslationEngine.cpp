#include "core/TranslationEngine.h"
#include "util/JsonHelper.h"
#include <QDebug>

TranslationEngine::TranslationEngine(std::unique_ptr<IGameAdapter> adapter)
    : m_adapter(std::move(adapter)) {}

bool TranslationEngine::detectGame(const QString& gameDir) const {
    return m_adapter->detect(gameDir);
}

IGameAdapter* TranslationEngine::adapter() const {
    return m_adapter.get();
}

std::vector<TranslationEntry> TranslationEngine::extractFromGame(const QString& gameDir) {
    m_entries = m_adapter->extractText(gameDir);
    return m_entries;
}

bool TranslationEngine::loadTranslation(const QString& filePath, QString* errorOut) {
    auto opt = JsonHelper::loadJsonFile(filePath);
    if (!opt.has_value()) {
        if (errorOut) *errorOut = "无法打开或解析 JSON 文件";
        return false;
    }
    auto& doc = *opt;
    
    nlohmann::json* stringsObj = nullptr;
    if (doc.contains("strings") && doc["strings"].is_object()) {
        // Format: {"strings": {"key": "value"}}
        stringsObj = &doc["strings"];
    } else if (doc.is_object()) {
        // Format: {"key": "value"} — flat key-value pairs
        stringsObj = &doc;
    } else {
        if (errorOut) *errorOut = "JSON 格式不正确（需为键值对对象）";
        return false;
    }
    
    m_translationMap.clear();
    int skipped = 0;
    for (auto& [key, value] : stringsObj->items()) {
        if (key == "meta" || key == "strings") continue; // skip metadata keys
        if (value.is_string()) {
            m_translationMap[key] = value.get<std::string>();
        } else {
            skipped++;
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
