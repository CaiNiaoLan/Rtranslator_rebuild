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
