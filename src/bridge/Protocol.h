#pragma once
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
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
