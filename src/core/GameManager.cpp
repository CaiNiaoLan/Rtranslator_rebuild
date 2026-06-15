#include "core/GameManager.h"
#include "bridge/Protocol.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTcpServer>

GameManager::GameManager(QObject* parent)
    : QObject(parent), m_process(new QProcess(this)), m_cdpClient(new CdpClient(this)),
      m_wsServer(new WebSocketServer(this)), m_cdpPort(9222), m_running(false), m_hookInjected(false),
      m_adapter(nullptr)
{
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GameManager::onGameProcessFinished);
    connect(m_cdpClient, &CdpClient::connected, this, &GameManager::onCdpConnected);
    connect(m_cdpClient, &CdpClient::disconnected, this, &GameManager::onCdpDisconnected);
    connect(m_wsServer, &WebSocketServer::clientConnected, this, &GameManager::onWsClientConnected);
    connect(m_wsServer, &WebSocketServer::clientDisconnected, this, &GameManager::onWsDisconnected);
}

void GameManager::setGamePath(const QString& path) { m_gamePath = path; }
QString GameManager::gamePath() const { return m_gamePath; }
bool GameManager::isRunning() const { return m_running; }
CdpClient* GameManager::cdpClient() const { return m_cdpClient; }
WebSocketServer* GameManager::wsServer() const { return m_wsServer; }
quint16 GameManager::cdpPort() const { return m_cdpPort; }

void GameManager::setTranslationMap(const std::unordered_map<std::string, std::string>& map) { m_translationMap = map; }

void GameManager::setAdapter(IGameAdapter* adapter) { m_adapter = adapter; }

bool GameManager::findAvailablePort() {
    for (int attempt = 0; attempt < 5; attempt++) {
        QTcpServer test;
        if (test.listen(QHostAddress::LocalHost, m_cdpPort)) { test.close(); return true; }
        m_cdpPort++;
    }
    return false;
}

void GameManager::launch() {
    if (m_gamePath.isEmpty()) { emit injectionFailed("No game path set"); return; }
    if (!m_wsServer->start(0)) { emit injectionFailed("Failed to start WebSocket server"); return; }
    if (!findAvailablePort()) { emit injectionFailed("No available CDP port"); return; }
    QStringList args;
    QString flag = m_adapter ? QString::fromStdString(m_adapter->debugPortFlag()) : "--remote-debugging-port=";
    args << QString("%1%2").arg(flag).arg(m_cdpPort);
    QFileInfo exeInfo(m_gamePath);
    m_process->setWorkingDirectory(exeInfo.absolutePath());
    m_process->start(m_gamePath, args);
    if (!m_process->waitForStarted(5000)) { emit injectionFailed("Failed to start game process"); return; }
    m_running = true;
    emit gameStarted();
    QTimer::singleShot(1000, this, [this]() { m_cdpClient->connectToDebugger("127.0.0.1", m_cdpPort); });
}

void GameManager::detach() {
    if (m_process && m_running) {
        m_process->closeWriteChannel();
        m_running = false; m_hookInjected = false;
        emit gameStopped();
    }
}

void GameManager::onCdpConnected() { injectHookScript(); }

void GameManager::onCdpDisconnected() {
    if (!m_hookInjected && m_running) emit injectionFailed("CDP connection failed");
}

void GameManager::onWsClientConnected() { m_hookInjected = true; sendInitMessage(); emit injectionSucceeded(); }
void GameManager::onWsDisconnected() { emit connectionLost(); }

void GameManager::injectHookScript() {
    injectSupportScript("ws-client.js", loadTextFile(":/inject/ws-client.js"));
    injectSupportScript("state-reader.js", loadTextFile(":/inject/state-reader.js"));
    injectSupportScript("translation.js", loadTextFile(":/inject/translation.js"));
    QString hook = loadTextFile(":/inject/hook.js");
    hook.replace("__RTRANSLATOR_WS_PORT__", QString::number(m_wsServer->port()));
    m_cdpClient->injectScript(hook);
}

void GameManager::injectSupportScript(const QString&, const QString& script) {
    if (!script.isEmpty()) m_cdpClient->injectScript(script);
}

void GameManager::sendInitMessage() {
    Protocol::InitCommand init;
    init.translationMap = m_translationMap;
    m_wsServer->sendMessage(QString::fromStdString(init.toJson().dump()));
}

void GameManager::onGameProcessFinished(int, QProcess::ExitStatus) {
    m_running = false; m_hookInjected = false; emit gameStopped();
}

QString GameManager::loadTextFile(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    return QString::fromUtf8(file.readAll());
}
