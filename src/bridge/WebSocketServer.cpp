#include "bridge/WebSocketServer.h"

WebSocketServer::WebSocketServer(QObject* parent)
    : QObject(parent), m_server(new QWebSocketServer("RTranslator", QWebSocketServer::NonSecureMode, this)), m_client(nullptr)
{
    connect(m_server, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
}

WebSocketServer::~WebSocketServer() { stop(); }

bool WebSocketServer::start(quint16 port) { return m_server->listen(QHostAddress::LocalHost, port); }
void WebSocketServer::stop() {
    if (m_client) { m_client->close(); m_client->deleteLater(); m_client = nullptr; }
    m_server->close();
}
quint16 WebSocketServer::port() const { return m_server->serverPort(); }
bool WebSocketServer::isListening() const { return m_server->isListening(); }

void WebSocketServer::sendMessage(const QString& message) {
    if (m_client && m_client->state() == QAbstractSocket::ConnectedState)
        m_client->sendTextMessage(message);
}
void WebSocketServer::setMessageHandler(MessageHandler handler) { m_handler = std::move(handler); }

void WebSocketServer::onNewConnection() {
    if (m_client) { auto* dup = m_server->nextPendingConnection(); dup->close(); dup->deleteLater(); return; }
    m_client = m_server->nextPendingConnection();
    connect(m_client, &QWebSocket::textMessageReceived, this, &WebSocketServer::onTextMessageReceived);
    connect(m_client, &QWebSocket::disconnected, this, &WebSocketServer::onSocketDisconnected);
    emit clientConnected();
}
void WebSocketServer::onTextMessageReceived(const QString& message) {
    emit messageReceived(message);
    if (m_handler) m_handler(message);
}
void WebSocketServer::onSocketDisconnected() { m_client->deleteLater(); m_client = nullptr; emit clientDisconnected(); }
