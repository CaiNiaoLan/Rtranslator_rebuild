#pragma once
#include <QObject>
#include <functional>
#include <nlohmann/json.hpp>

class CheatController : public QObject {
    Q_OBJECT
public:
    using SendHandler = std::function<void(const QString& json)>;
    explicit CheatController(QObject* parent = nullptr);
    void setSendHandler(SendHandler handler);
    void readState(const std::string& path);
    void writeState(const std::string& path, const nlohmann::json& value);
    void batchWrite(const std::vector<std::pair<std::string, nlohmann::json>>& ops);
    void battleVictory();
    void addItem(int id, const std::string& type, int count);
    void handleMessage(const QString& message);
signals:
    void stateUpdated(const QString& path, const QString& value);
    void operationFailed(const QString& reason);
    void operationAcknowledged(const QString& cmd, bool success);
private:
    SendHandler m_sendHandler;
    void sendCommand(const std::string& json);
};
