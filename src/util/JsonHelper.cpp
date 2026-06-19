#include "util/JsonHelper.h"
#include <QFile>
#include <QTextStream>

namespace JsonHelper {

std::optional<json> loadJsonFile(const QString& filepath) {
    QFile file(filepath);
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
