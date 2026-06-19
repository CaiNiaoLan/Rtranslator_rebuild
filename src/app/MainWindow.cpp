#include "app/MainWindow.h"
#include "bridge/Protocol.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlContext>

MainWindow::MainWindow(QQmlApplicationEngine* engine, QObject* parent)
    : QObject(parent), m_engine(engine),
      m_transEngine(std::make_unique<TranslationEngine>(std::make_unique<RpgMVAdapter>())),
      m_gameManager(std::make_unique<GameManager>(this)),
      m_cheatController(std::make_unique<CheatController>(this)),
      m_connected(false),
      m_statusText("就绪 — 请选择游戏目录")
{
    connect(m_gameManager.get(), &GameManager::gameStarted, this, [this]() {
        m_statusText = "游戏已启动，正在注入..."; emit statusTextChanged();
        emit gameStarted();
    });
    connect(m_gameManager.get(), &GameManager::gameStopped, this, &MainWindow::gameStopped);
    connect(m_gameManager.get(), &GameManager::injectionSucceeded, this, &MainWindow::onInjectionSucceeded);
    connect(m_gameManager.get(), &GameManager::injectionFailed, this, &MainWindow::onInjectionFailed);
    connect(m_gameManager.get(), &GameManager::connectionLost, this, [this]() {
        m_connected = false; m_statusText = "连接已断开";
        emit statusTextChanged(); emit connectionChanged();
    });
    m_gameManager->setAdapter(m_transEngine->adapter());
    m_cheatController->setSendHandler([this](const QString& json) {
        if (m_gameManager->wsServer()) m_gameManager->wsServer()->sendMessage(json);
    });
    if (m_gameManager->wsServer()) {
        m_gameManager->wsServer()->setMessageHandler([this](const QString& msg) {
            m_cheatController->handleMessage(msg);
        });
    }
    connect(m_cheatController.get(), &CheatController::notificationReceived, this,
        [this](const QString& event, const QJsonObject& data) {
            Q_UNUSED(data);
            if (event == "hooks_installed") {
                m_statusText = "翻译钩子已安装"; emit statusTextChanged();
            } else if (event == "init_received") {
                int count = data.value("count").toInt();
                m_statusText = QString("游戏端收到 %1 条翻译").arg(count); emit statusTextChanged();
            }
        });
}

QString MainWindow::gamePath() const { return m_gamePath; }
void MainWindow::setGamePath(const QString& path) {
    if (m_gamePath == path) return;
    m_gamePath = path; emit gamePathChanged();
}
bool MainWindow::isConnected() const { return m_connected; }
QString MainWindow::statusText() const { return m_statusText; }
QString MainWindow::translationStats() const { return m_transStats; }
QString MainWindow::currentMapName() const { return m_currentMapName; }
QString MainWindow::currentMapId() const { return m_currentMapId; }
QString MainWindow::playerX() const { return m_playerX; }
QString MainWindow::playerY() const { return m_playerY; }
QString MainWindow::selectedEventName() const { return m_selectedEventName; }
QString MainWindow::selectedEventDetail() const { return m_selectedEventDetail; }
int MainWindow::selectedEventId() const { return m_selectedEventId; }
QVariantList MainWindow::mapListModel() const { return m_mapListModel; }
QVariantList MainWindow::switchListModel() const { return m_switchListModel; }
QVariantList MainWindow::variableListModel() const { return m_variableListModel; }
QVariantList MainWindow::commonEventListModel() const { return m_commonEventListModel; }
QVariantList MainWindow::mapEventListModel() const { return m_mapEventListModel; }
QVariantList MainWindow::partyStatusModel() const { return m_partyStatusModel; }
QVariantMap MainWindow::currentMapInfo() const { return m_currentMapInfo; }

void MainWindow::launchGame() { m_gameManager->setGamePath(m_gamePath); m_gameManager->launch(); }
void MainWindow::detachGame() { m_gameManager->detach(); }

void MainWindow::loadGameDir(const QString& dir) {
    QDir gameDir(dir);
    QStringList exes = gameDir.entryList({"*.exe"}, QDir::Files);
    QString foundExe;
    for (const auto& exe : exes) {
        if (exe.compare("Game.exe", Qt::CaseInsensitive) == 0 || exe.contains("nw", Qt::CaseInsensitive)) {
            foundExe = gameDir.absoluteFilePath(exe);
            break;
        }
    }
    if (!foundExe.isEmpty()) {
        m_gamePath = foundExe; emit gamePathChanged();
        if (m_transEngine->detectGame(dir)) {
            m_statusText = "已检测到游戏: " + dir; emit statusTextChanged();
        } else {
            m_statusText = "游戏已选择（未检测到数据目录，翻译和修改功能可能受限）"; emit statusTextChanged();
        }
    } else if (!m_gamePath.isEmpty()) {
        // Game was set by browseGameFile directly
        if (m_transEngine->detectGame(dir)) {
            m_statusText = "已检测到游戏: " + dir; emit statusTextChanged();
        }
    }
}

void MainWindow::loadTranslationFile(const QString& filePath) {
    if (filePath.isEmpty()) {
        m_statusText = "请先选择翻译文件"; emit statusTextChanged();
        return;
    }
    QString error;
    if (m_transEngine->loadTranslation(filePath, &error)) {
        int count = static_cast<int>(m_transEngine->translationMap().size());
        m_transStats = QString("已载入 %1 条翻译").arg(count);
        emit translationStatsChanged(); emit translationLoaded(count);
        m_gameManager->setTranslationMap(m_transEngine->translationMap());
        // If game is already connected, push translation immediately
        m_gameManager->sendTranslation();
        m_statusText = QString("翻译就绪: %1 条").arg(count); emit statusTextChanged();
    } else {
        m_statusText = error.isEmpty() ? "加载失败" : error; emit statusTextChanged();
    }
}

void MainWindow::extractText() {
    if (m_gamePath.isEmpty()) {
        m_statusText = "请先选择游戏目录"; emit statusTextChanged(); return;
    }
    QFileInfo info(m_gamePath);
    auto entries = m_transEngine->extractFromGame(info.absolutePath());
    QVariantList list;
    for (const auto& e : entries) {
        QVariantMap map;
        map["id"] = QString::fromStdString(e.id);
        map["source"] = QString::fromStdString(e.source);
        map["context"] = QString::fromStdString(e.context);
        map["original"] = QString::fromStdString(e.original);
        list.append(map);
    }
    m_transStats = QString("已提取 %1 条原文").arg(list.size());
    emit translationStatsChanged(); emit extractionComplete(list);
    m_statusText = QString("已提取 %1 条原文").arg(list.size()); emit statusTextChanged();
}

void MainWindow::exportOriginalText() {
    if (m_gamePath.isEmpty()) {
        m_statusText = "请先选择游戏目录"; emit statusTextChanged(); return;
    }
    QFileInfo info(m_gamePath);
    auto entries = m_transEngine->extractFromGame(info.absolutePath());

    nlohmann::json output = nlohmann::json::object();
    for (const auto& e : entries) {
        output[e.original] = "";
    }

    QString outPath = info.absolutePath() + "/rtranslator_export.json";
    QFile file(outPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        stream << QString::fromStdString(output.dump(2));
        file.close();
        m_statusText = "已导出: " + outPath; emit statusTextChanged();
        m_transStats = QString("已导出 %1 条原文").arg(entries.size()); emit translationStatsChanged();
    } else {
        m_statusText = "导出失败，无法写入文件"; emit statusTextChanged();
    }
}

void MainWindow::readGameState(const QString& path) { m_cheatController->readState(path.toStdString()); }
void MainWindow::writeGameState(const QString& path, const QVariant& value) {
    nlohmann::json j;
    int type = static_cast<int>(value.type());
    if (type == static_cast<int>(QVariant::Bool)) j = value.toBool();
    else if (type == static_cast<int>(QVariant::Int)) j = value.toInt();
    else if (type == static_cast<int>(QVariant::Double)) j = value.toDouble();
    else j = value.toString().toStdString();
    m_cheatController->writeState(path.toStdString(), j);
}
void MainWindow::triggerBattleVictory() { m_cheatController->battleVictory(); }
void MainWindow::addItemToInventory(int id, const QString& type, int count) { m_cheatController->addItem(id, type.toStdString(), count); }

void MainWindow::cheatAction(const QString& action) {
    m_cheatController->sendCheat(action);
}

QString MainWindow::browseGameFile() {
    QString path = QFileDialog::getOpenFileName(nullptr, "选择游戏可执行文件", QString(),
        "游戏程序 (*.exe);;所有文件 (*)");
    if (!path.isEmpty()) {
        setGamePath(path);
        QFileInfo info(path);
        loadGameDir(info.absolutePath());
    }
    return path;
}

QString MainWindow::browseTranslationFile() {
    qDebug() << "browseTranslationFile called";
    QString path = QFileDialog::getOpenFileName(nullptr, "选择翻译文件", QString(),
        "JSON 文件 (*.json);;所有文件 (*)");
    qDebug() << "browseTranslationFile returned:" << path;
    if (!path.isEmpty()) {
        m_statusText = "已选择: " + path; emit statusTextChanged();
    }
    return path;
}

void MainWindow::onInjectionSucceeded() { m_connected = true; m_statusText = "已连接"; emit statusTextChanged(); emit connectionChanged(); }
void MainWindow::onInjectionFailed(const QString& reason) { m_connected = false; m_statusText = "连接失败: " + reason; emit statusTextChanged(); emit connectionChanged(); }

// ---- Tab data stubs ----
void MainWindow::teleportToMap(int mapId) {
    m_cheatController->writeState("player.mapId", mapId);
}

void MainWindow::toggleSwitch(int id, bool on) {
    QString path = QString("switches.%1").arg(id);
    writeGameState(path, QVariant(on));
}

void MainWindow::setVariable(int id, int value) {
    QString path = QString("variables.%1").arg(id);
    writeGameState(path, QVariant(value));
}

void MainWindow::selectCommonEvent(int id) {
    m_selectedEventId = id;
    m_selectedEventName = QString("\u516C\u5171\u4E8B\u4EF6 #%1").arg(id);
    m_selectedEventDetail = QString("\u4E8B\u4EF6 ID: %1\n\u7C7B\u578B: \u516C\u5171\u4E8B\u4EF6\n\n\u529F\u80FD\u5F00\u53D1\u4E2D...").arg(id);
    emit selectedEventNameChanged(); emit selectedEventDetailChanged();
    emit selectedEventIdChanged();
}

void MainWindow::selectMapEvent(int id) {
    m_selectedEventId = id;
    m_selectedEventName = QString("\u5730\u56FE\u4E8B\u4EF6 #%1").arg(id);
    m_selectedEventDetail = QString("\u4E8B\u4EF6 ID: %1\n\u7C7B\u578B: \u5730\u56FE\u4E8B\u4EF6\n\n\u529F\u80FD\u5F00\u53D1\u4E2D...").arg(id);
    emit selectedEventNameChanged(); emit selectedEventDetailChanged();
    emit selectedEventIdChanged();
}

void MainWindow::triggerSelectedEvent() {
    m_cheatController->sendCheat(QString("trigger_event_%1").arg(m_selectedEventId));
}
