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
