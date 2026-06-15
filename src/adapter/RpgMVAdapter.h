#pragma once
#include "adapter/IGameAdapter.h"
#include <nlohmann/json.hpp>

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
