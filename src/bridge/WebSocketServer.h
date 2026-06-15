#pragma once
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <functional>

class WebSocketServer : public QObject {
    Q_OBJECT
public:
    using MessageHandler = std::function<void(const QString& message)>;
    explicit WebSocketServer(QObject* parent = nullptr);
    ~WebSocketServer();
    bool start(quint16 port = 0);
    void stop();
    quint16 port() const;
    bool isListening() const;
    void sendMessage(const QString& message);
    void setMessageHandler(MessageHandler handler);
signals:
    void clientConnected();
    void clientDisconnected();
    void messageReceived(const QString& message);
private slots:
    void onNewConnection();
    void onTextMessageReceived(const QString& message);
    void onSocketDisconnected();
private:
    QWebSocketServer* m_server;
    QWebSocket* m_client;
    MessageHandler m_handler;
};
