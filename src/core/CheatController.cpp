#include "core/CheatController.h"
#include "bridge/Protocol.h"
#include <QJsonDocument>
#include <QJsonObject>

CheatController::CheatController(QObject* parent) : QObject(parent) {}
void CheatController::setSendHandler(SendHandler handler) { m_sendHandler = std::move(handler); }
void CheatController::sendCommand(const std::string& json) { if (m_sendHandler) m_sendHandler(QString::fromStdString(json)); }
void CheatController::readState(const std::string& path) { sendCommand(Protocol::ReadCommand{path}.toJson().dump()); }
void CheatController::writeState(const std::string& path, const nlohmann::json& value) { sendCommand(Protocol::WriteCommand{path, value}.toJson().dump()); }
void CheatController::batchWrite(const std::vector<std::pair<std::string, nlohmann::json>>& ops) {
    Protocol::BatchCommand batch;
    for (const auto& [p, v] : ops) batch.ops.push_back(Protocol::WriteCommand{p, v}.toJson());
    sendCommand(batch.toJson().dump());
}
void CheatController::battleVictory() { sendCommand(Protocol::BattleVictoryCommand{}.toJson().dump()); }
void CheatController::addItem(int id, const std::string& type, int count) { sendCommand(Protocol::AddItemCommand{id, type, count}.toJson().dump()); }

void CheatController::handleMessage(const QString& message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();
    QString type = obj.value("type").toString();
    if (type == "state") {
        emit stateUpdated(obj.value("path").toString(),
            QJsonDocument(obj.value("value").toObject()).toJson(QJsonDocument::Compact));
    } else if (type == "error") {
        emit operationFailed(obj.value("reason").toString());
    } else if (type == "ack") {
        bool success = obj.value("success").toBool();
        QString cmd = obj.value("cmd").toString();
        emit operationAcknowledged(cmd, success);
    }
}
