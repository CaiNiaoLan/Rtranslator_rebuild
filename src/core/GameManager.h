#pragma once
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <memory>
#include <unordered_map>
#include "bridge/WebSocketServer.h"
#include "adapter/IGameAdapter.h"

class GameManager : public QObject {
    Q_OBJECT
public:
    explicit GameManager(QObject* parent = nullptr);
    void setGamePath(const QString& path);
    QString gamePath() const;
    void launch();
    void detach();
    bool isRunning() const;
    WebSocketServer* wsServer() const;
    void setTranslationMap(const std::unordered_map<std::string, std::string>& map);
    void sendTranslation();
    void setAdapter(IGameAdapter* adapter);

signals:
    void gameStarted();
    void gameStopped();
    void injectionSucceeded();
    void injectionFailed(const QString& reason);
    void connectionLost();

private slots:
    void onWsClientConnected();
    void onWsDisconnected();
    void onGameProcessFinished(int exitCode, QProcess::ExitStatus status);

private:
    bool prepareInjection();
    void restoreInjection();
    QString buildInjectedHtml(const QString& original);
    QString loadTextFile(const QString& path) const;
    void sendInitMessage();

    QProcess* m_process;
    WebSocketServer* m_wsServer;
    QString m_gamePath;
    QString m_gameDir;
    bool m_running;
    bool m_hookInjected;
    std::unordered_map<std::string, std::string> m_translationMap;
    IGameAdapter* m_adapter;
    QString m_htmlBackup;
    QString m_htmlPath;
};
