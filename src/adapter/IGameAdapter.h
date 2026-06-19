#pragma once
#include <string>
#include <vector>
#include <optional>
#include <QString>

struct TranslationEntry {
    std::string id;
    std::string source;
    std::string context;
    std::string field;
    std::string original;
    std::string translation;
};

class IGameAdapter {
public:
    virtual ~IGameAdapter() = default;
    virtual bool detect(const QString& gameDir) = 0;
    virtual std::vector<TranslationEntry> extractText(const QString& gameDir) = 0;
    virtual std::string executableName() const = 0;
    virtual std::string engineName() const = 0;
    virtual std::string debugPortFlag() const { return "--remote-debugging-port="; }
};
