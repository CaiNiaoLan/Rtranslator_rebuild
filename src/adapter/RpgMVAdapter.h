#pragma once
#include "adapter/IGameAdapter.h"
#include <nlohmann/json.hpp>

class RpgMVAdapter : public IGameAdapter {
public:
    bool detect(const QString& gameDir) override;
    std::vector<TranslationEntry> extractText(const QString& gameDir) override;
    std::string executableName() const override { return "Game.exe"; }
    std::string engineName() const override { return "RPG Maker MV"; }

private:
    void extractFromFile(const QString& filePath, const QString& source,
                         const QString& contextPrefix,
                         const std::vector<std::string>& fields,
                         std::vector<TranslationEntry>& out);
    void extractFromMapFile(const QString& filePath, const QString& source,
                            std::vector<TranslationEntry>& out);
    void extractFromSystemFile(const QString& filePath, const QString& source,
                               std::vector<TranslationEntry>& out);
    void extractEventParameters(const nlohmann::json& list, const std::string& source,
                                const std::string& context, int eventId, int pageIdx,
                                std::vector<TranslationEntry>& out);
};
