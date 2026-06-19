#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <QString>
#include "adapter/IGameAdapter.h"

class TranslationEngine {
public:
    explicit TranslationEngine(std::unique_ptr<IGameAdapter> adapter);
    bool detectGame(const QString& gameDir) const;
    std::vector<TranslationEntry> extractFromGame(const QString& gameDir);
    bool loadTranslation(const QString& filePath, QString* errorOut = nullptr);
    std::string translate(const std::string& original) const;
    const std::unordered_map<std::string, std::string>& translationMap() const;
    IGameAdapter* adapter() const;
    std::unordered_set<std::string> buildWhitelist(const std::vector<TranslationEntry>& entries) const;
private:
    std::unique_ptr<IGameAdapter> m_adapter;
    std::unordered_map<std::string, std::string> m_translationMap;
    std::vector<TranslationEntry> m_entries;
};
