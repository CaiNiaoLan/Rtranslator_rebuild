#include "bridge/CdpClient.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>

CdpClient::CdpClient(QObject* parent)
    : QObject(parent), m_nam(new QNetworkAccessManager(this)),
      m_ws(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this)),
      m_timeout(new QTimer(this)), m_port(0), m_cmdId(0)
{
    m_timeout->setSingleShot(true);
    m_timeout->setInterval(3000);
    connect(m_ws, &QWebSocket::connected, this, &CdpClient::onWsConnected);
    connect(m_ws, &QWebSocket::disconnected, this, &CdpClient::onWsDisconnected);
    connect(m_ws, &QWebSocket::textMessageReceived, this, &CdpClient::onWsTextMessage);
    connect(m_timeout, &QTimer::timeout, this, &CdpClient::onConnectionTimeout);
}

void CdpClient::connectToDebugger(const QString& host, quint16 port) {
    m_host = host; m_port = port;
    QUrl url(QString("http://%1:%2/json").arg(host).arg(port));
    auto* reply = m_nam->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { onHttpResponse(reply); });
    m_timeout->start();
}

void CdpClient::onHttpResponse(QNetworkReply* reply) {
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) { emit disconnected(); return; }
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isArray()) { emit disconnected(); return; }
    for (const auto& page : doc.array()) {
        QJsonObject obj = page.toObject();
        if (obj.value("type").toString() == "page") {
            QString wsUrl = obj.value("webSocketDebuggerUrl").toString();
            if (!wsUrl.isEmpty()) { m_timeout->stop(); connectToPage(wsUrl); return; }
        }
    }
    emit disconnected();
}

void CdpClient::connectToPage(const QString& wsUrl) { m_ws->open(QUrl(wsUrl)); }

void CdpClient::injectScript(const QString& jsCode) {
    if (!isConnected()) return;
    m_cmdId++;
    QJsonObject cmd;
    cmd["id"] = m_cmdId; cmd["method"] = "Runtime.evaluate";
    QJsonObject params; params["expression"] = jsCode; params["awaitPromise"] = false;
    cmd["params"] = params;
    m_ws->sendTextMessage(QJsonDocument(cmd).toJson(QJsonDocument::Compact));
    emit injectionComplete();
}

bool CdpClient::isConnected() const { return m_ws->state() == QAbstractSocket::ConnectedState; }
void CdpClient::setDisconnectHandler(DisconnectHandler handler) { m_disconnectHandler = std::move(handler); }
void CdpClient::onWsConnected() { emit connected(); }
void CdpClient::onWsDisconnected() { if (m_disconnectHandler) m_disconnectHandler(); emit disconnected(); }
void CdpClient::onWsTextMessage(const QString& msg) { Q_UNUSED(msg); }
void CdpClient::onConnectionTimeout() { emit disconnected(); }
