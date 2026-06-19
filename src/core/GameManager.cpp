#include "core/GameManager.h"
#include "bridge/Protocol.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTcpServer>

GameManager::GameManager(QObject* parent)
    : QObject(parent), m_process(new QProcess(this)),
      m_wsServer(new WebSocketServer(this)),
      m_running(false), m_hookInjected(false), m_adapter(nullptr)
{
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &GameManager::onGameProcessFinished);
    connect(m_wsServer, &WebSocketServer::clientConnected, this, &GameManager::onWsClientConnected);
    connect(m_wsServer, &WebSocketServer::clientDisconnected, this, &GameManager::onWsDisconnected);
}

void GameManager::setGamePath(const QString& path) {
    m_gamePath = path;
    QFileInfo info(path);
    m_gameDir = info.absolutePath();
}

QString GameManager::gamePath() const { return m_gamePath; }
bool GameManager::isRunning() const { return m_running; }
WebSocketServer* GameManager::wsServer() const { return m_wsServer; }
void GameManager::setAdapter(IGameAdapter* adapter) { m_adapter = adapter; }

void GameManager::setTranslationMap(const std::unordered_map<std::string, std::string>& map) {
    m_translationMap = map;
}

void GameManager::sendTranslation() {
    Protocol::ReloadTransCommand cmd;
    cmd.translationMap = m_translationMap;
    m_wsServer->sendMessage(QString::fromStdString(cmd.toJson().dump()));
}

bool GameManager::prepareInjection() {
    // Find index.html in common game layouts
    QStringList candidates = {
        m_gameDir + "/www/index.html",
        m_gameDir + "/index.html",
        m_gameDir + "/www/data/../index.html"
    };
    
    m_htmlPath.clear();
    for (const auto& path : candidates) {
        if (QFileInfo::exists(path)) {
            m_htmlPath = path;
            break;
        }
    }
    
    if (m_htmlPath.isEmpty()) {
        qWarning() << "RTranslator: index.html not found in" << m_gameDir;
        emit injectionFailed("未找到 index.html，无法注入翻译脚本\n检查过: " + candidates.join(", "));
        return false;
    }

    qDebug() << "RTranslator: injecting into" << m_htmlPath << "mapSize=" << m_translationMap.size();

    // Read original HTML
    QFile file(m_htmlPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit injectionFailed("无法读取 " + m_htmlPath);
        return false;
    }
    m_htmlBackup = QString::fromUtf8(file.readAll());
    file.close();

    // Build injected HTML
    QString injected = buildInjectedHtml(m_htmlBackup);

    // Write modified HTML
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        emit injectionFailed("无法写入 " + m_htmlPath);
        return false;
    }
    QTextStream stream(&file);
    stream.setCodec("UTF-8");
    stream << injected;
    file.close();

    return true;
}

void GameManager::restoreInjection() {
    if (m_htmlPath.isEmpty() || m_htmlBackup.isEmpty()) return;
    QFile file(m_htmlPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream << m_htmlBackup;
        file.close();
    }
}

QString GameManager::buildInjectedHtml(const QString& original) {
    // Load all inject scripts
    QString wsClient = loadTextFile(":/inject/ws-client.js");
    QString stateReader = loadTextFile(":/inject/state-reader.js");
    QString translation = loadTextFile(":/inject/translation.js");
    QString hook = loadTextFile(":/inject/hook.js");
    hook.replace("__RTRANSLATOR_WS_PORT__", QString::number(m_wsServer->port()));

    // Embed translation map so it's available synchronously before JSON.parse hooks fire
    QString preload;
    if (!m_translationMap.empty()) {
        nlohmann::json mapJson(m_translationMap);
        QString jsonStr = QString::fromStdString(mapJson.dump());
        jsonStr.replace("</", "<\\/");
        preload = "window.__RTRANSLATOR_PRELOAD_MAP__=" + jsonStr + ";\n";
    }

    QString scriptBlock =
        QString("\n<!-- RTranslator Injection -->\n<script>\n") +
        QString("document.title='RTL:LOADED';\n") +
        QString("window.__RTRANSLATOR_ERRLOG__=[];\n") +
        preload +
        QString("try{") + wsClient + QString("}catch(e){window.__RTRANSLATOR_ERRLOG__.push('WS:'+e.message);}\n") +
        QString("try{") + stateReader + QString("}catch(e){window.__RTRANSLATOR_ERRLOG__.push('STATE:'+e.message);}\n") +
        QString("try{") + translation + QString("}catch(e){window.__RTRANSLATOR_ERRLOG__.push('TRANS:'+e.message);}\n") +
        QString("try{") + hook + QString("}catch(e){window.__RTRANSLATOR_ERRLOG__.push('HOOK:'+e.message);}\n") +
        QString("var ms=window.__RTRANSLATOR_PRELOAD_MAP__?Object.keys(window.__RTRANSLATOR_PRELOAD_MAP__).length:0;\n") +
        QString("document.title='RTL:MAP'+ms;\n") +
        QString("window.__RTRANSLATOR_ERRLOG__.push('DONE map='+ms);\n") +
        QString("try{if(typeof require!=='undefined'){var fs=require('fs');fs.writeFileSync('rtranslator_err.log',window.__RTRANSLATOR_ERRLOG__.join('\\n'),'utf-8');}}catch(e){}\n") +
        QString("\n</script>\n<!-- End RTranslator -->\n");

    // Insert before </body> or </html> or at the end
    if (original.contains("</body>")) {
        return QString(original).replace("</body>", scriptBlock + "</body>");
    } else if (original.contains("</html>")) {
        return QString(original).replace("</html>", scriptBlock + "</html>");
    } else {
        return original + scriptBlock;
    }
}

void GameManager::launch() {
    if (m_gamePath.isEmpty()) { emit injectionFailed("未设置游戏路径"); return; }

    // Ensure WebSocket server is running
    if (!m_wsServer->isListening()) {
        if (!m_wsServer->start(0)) {
            emit injectionFailed("无法启动本地服务"); return;
        }
    }

    // Prepare file-based injection
    if (!prepareInjection()) return;

    // Launch game
    m_process->setWorkingDirectory(m_gameDir);
    m_process->start(m_gamePath, QStringList());

    connect(m_process, &QProcess::started, this, [this]() {
        m_running = true;
        emit gameStarted();
    });
    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        if (!m_running) {
            restoreInjection();
            emit injectionFailed("无法启动游戏进程");
        }
    });
}

void GameManager::detach() {
    if (m_process && m_running) {
        restoreInjection();
        m_process->terminate();
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
        }
        m_running = false;
        m_hookInjected = false;
        emit gameStopped();
    }
}

void GameManager::onWsClientConnected() {
    m_hookInjected = true;
    sendInitMessage();
    emit injectionSucceeded();
}

void GameManager::onWsDisconnected() {
    emit connectionLost();
}

void GameManager::sendInitMessage() {
    Protocol::InitCommand init;
    init.translationMap = m_translationMap;
    m_wsServer->sendMessage(QString::fromStdString(init.toJson().dump()));
}

void GameManager::onGameProcessFinished(int, QProcess::ExitStatus) {
    restoreInjection();
    m_running = false;
    m_hookInjected = false;
    emit gameStopped();
}

QString GameManager::loadTextFile(const QString& path) const {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    return QString::fromUtf8(file.readAll());
}
