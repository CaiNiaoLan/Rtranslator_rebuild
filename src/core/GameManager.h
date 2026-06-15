#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <memory>
#include <unordered_map>
#include "bridge/CdpClient.h"
#include "bridge/WebSocketServer.h"

class GameManager : public QObject {
    Q_OBJECT
public:
    explicit GameManager(QObject* parent = nullptr);
    void setGamePath(const QString& path);
    QString gamePath() const;
    void launch();
    void detach();
    bool isRunning() const;
    CdpClient* cdpClient() const;
    WebSocketServer* wsServer() const;
    quint16 cdpPort() const;
    void setTranslationMap(const std::unordered_map<std::string, std::string>& map);
signals:
    void gameStarted();
    void gameStopped();
    void injectionSucceeded();
    void injectionFailed(const QString& reason);
    void connectionLost();
private slots:
    void onCdpConnected();
    void onCdpDisconnected();
    void onWsClientConnected();
    void onWsDisconnected();
    void onGameProcessFinished(int exitCode, QProcess::ExitStatus status);
private:
    void injectHookScript();
    void injectSupportScript(const QString& name, const QString& script);
    void sendInitMessage();
    QString loadTextFile(const QString& path) const;
    bool findAvailablePort();
    QProcess* m_process;
    CdpClient* m_cdpClient;
    WebSocketServer* m_wsServer;
    QString m_gamePath;
    quint16 m_cdpPort;
    bool m_running;
    bool m_hookInjected;
    std::unordered_map<std::string, std::string> m_translationMap;
};
