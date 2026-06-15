#include "app/MainWindow.h"
#include "bridge/Protocol.h"
#include <QDir>
#include <QFileInfo>
#include <QQmlContext>

MainWindow::MainWindow(QQmlApplicationEngine* engine, QObject* parent)
    : QObject(parent), m_engine(engine),
      m_transEngine(std::make_unique<TranslationEngine>(std::make_unique<RpgMVAdapter>())),
      m_gameManager(std::make_unique<GameManager>(this)),
      m_cheatController(std::make_unique<CheatController>(this)),
      m_connected(false),
      m_statusText("就绪 — 请选择游戏目录")
{
    connect(m_gameManager.get(), &GameManager::gameStarted, this, &MainWindow::gameStarted);
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
}

QString MainWindow::gamePath() const { return m_gamePath; }
void MainWindow::setGamePath(const QString& path) { m_gamePath = path; emit gamePathChanged(); }
bool MainWindow::isConnected() const { return m_connected; }
QString MainWindow::statusText() const { return m_statusText; }
QString MainWindow::translationStats() const { return m_transStats; }

void MainWindow::launchGame() { m_gameManager->setGamePath(m_gamePath); m_gameManager->launch(); }
void MainWindow::detachGame() { m_gameManager->detach(); }

void MainWindow::loadGameDir(const QString& dir) {
    if (!m_transEngine->detectGame(dir.toStdString())) {
        m_statusText = "不是有效的 RPG Maker MV/MZ 游戏"; emit statusTextChanged(); return;
    }
    QDir gameDir(dir);
    QStringList exes = gameDir.entryList({"*.exe"}, QDir::Files);
    for (const auto& exe : exes) {
        if (exe.compare("Game.exe", Qt::CaseInsensitive) == 0 || exe.contains("nw", Qt::CaseInsensitive)) {
            m_gamePath = gameDir.absoluteFilePath(exe); emit gamePathChanged();
            m_statusText = "已检测到游戏: " + dir; emit statusTextChanged(); return;
        }
    }
    m_statusText = "未找到游戏可执行文件"; emit statusTextChanged();
}

void MainWindow::loadTranslationFile(const QString& filePath) {
    if (m_gamePath.isEmpty()) {
        m_statusText = "请先选择游戏路径"; emit statusTextChanged(); return;
    }
    if (m_transEngine->loadTranslation(filePath.toStdString())) {
        int count = static_cast<int>(m_transEngine->translationMap().size());
        m_transStats = QString("已载入 %1 条翻译").arg(count);
        emit translationStatsChanged(); emit translationLoaded(count);
        m_gameManager->setTranslationMap(m_transEngine->translationMap());
        
        // Sync with panel entries
        m_entries = m_transEngine->extractFromGame(QFileInfo(m_gamePath).absolutePath().toStdString());
        const auto& map = m_transEngine->translationMap();
        for (auto& entry : m_entries) {
            auto it = map.find(entry.original);
            if (it != map.end()) entry.translation = it->second;
        }
        // Re-emit to update panel
        QVariantList list;
        for (const auto& e : m_entries) {
            QVariantMap m;
            m["id"] = QString::fromStdString(e.id);
            m["source"] = QString::fromStdString(e.source);
            m["context"] = QString::fromStdString(e.context);
            m["original"] = QString::fromStdString(e.original);
            m["translation"] = QString::fromStdString(e.translation);
            list.append(m);
        }
        emit extractionComplete(list);
        
        if (m_gameManager->wsServer()) {
            Protocol::ReloadTransCommand cmd; cmd.translationMap = m_transEngine->translationMap();
            m_gameManager->wsServer()->sendMessage(QString::fromStdString(cmd.toJson().dump()));
        }
    } else {
        m_statusText = "翻译文件加载失败"; emit statusTextChanged();
    }
}

void MainWindow::extractText() {
    if (m_gamePath.isEmpty()) {
        m_statusText = "请先选择游戏路径"; emit statusTextChanged(); return;
    }
    QFileInfo info(m_gamePath);
    auto entries = m_transEngine->extractFromGame(info.absolutePath().toStdString());
    QVariantList list;
    for (const auto& e : entries) {
        QVariantMap map;
        map["id"] = QString::fromStdString(e.id);
        map["source"] = QString::fromStdString(e.source);
        map["context"] = QString::fromStdString(e.context);
        map["original"] = QString::fromStdString(e.original);
        map["translation"] = QString::fromStdString(e.translation);
        list.append(map);
    }
    m_transStats = QString("已提取 %1 条文本").arg(list.size());
    emit translationStatsChanged(); emit extractionComplete(list);
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

void MainWindow::onInjectionSucceeded() { m_connected = true; m_statusText = "已连接"; emit statusTextChanged(); emit connectionChanged(); }
void MainWindow::onInjectionFailed(const QString& reason) { m_connected = false; m_statusText = "连接失败: " + reason; emit statusTextChanged(); emit connectionChanged(); }
