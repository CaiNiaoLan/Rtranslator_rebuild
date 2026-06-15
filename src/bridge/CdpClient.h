#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QWebSocket>
#include <QTimer>
#include <functional>

class CdpClient : public QObject {
    Q_OBJECT
public:
    using DisconnectHandler = std::function<void()>;
    explicit CdpClient(QObject* parent = nullptr);
    void connectToDebugger(const QString& host, quint16 port);
    void injectScript(const QString& jsCode);
    bool isConnected() const;
    void setDisconnectHandler(DisconnectHandler handler);
signals:
    void connected();
    void disconnected();
    void injectionComplete();
private slots:
    void onHttpResponse(QNetworkReply* reply);
    void onWsConnected();
    void onWsDisconnected();
    void onWsTextMessage(const QString& message);
    void onConnectionTimeout();
private:
    void connectToPage(const QString& wsUrl);
    QNetworkAccessManager* m_nam;
    QWebSocket* m_ws;
    QTimer* m_timeout;
    QString m_host;
    quint16 m_port;
    int m_cmdId;
    DisconnectHandler m_disconnectHandler;
};
