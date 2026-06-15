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
