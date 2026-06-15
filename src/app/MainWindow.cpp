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
      m_connected(false)
{
    connect(m_gameManager.get(), &GameManager::gameStarted, this, &MainWindow::gameStarted);
    connect(m_gameManager.get(), &GameManager::gameStopped, this, &MainWindow::gameStopped);
    connect(m_gameManager.get(), &GameManager::injectionSucceeded, this, &MainWindow::onInjectionSucceeded);
    connect(m_gameManager.get(), &GameManager::injectionFailed, this, &MainWindow::onInjectionFailed);
    connect(m_gameManager.get(), &GameManager::connectionLost, this, [this]() {
        m_connected = false; m_statusText = "Connection lost";
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
        m_statusText = "Not a valid RPG Maker MV/MZ game"; emit statusTextChanged(); return;
    }
    QDir gameDir(dir);
    QStringList exes = gameDir.entryList({"*.exe"}, QDir::Files);
    for (const auto& exe : exes) {
        if (exe.compare("Game.exe", Qt::CaseInsensitive) == 0 || exe.contains("nw", Qt::CaseInsensitive)) {
            m_gamePath = gameDir.absoluteFilePath(exe); emit gamePathChanged();
            m_statusText = "Game detected: " + dir; emit statusTextChanged(); return;
        }
    }
    m_statusText = "No game executable found"; emit statusTextChanged();
}

void MainWindow::loadTranslationFile(const QString& filePath) {
    if (m_transEngine->loadTranslation(filePath.toStdString())) {
        int count = static_cast<int>(m_transEngine->translationMap().size());
        m_transStats = QString("%1 translations loaded").arg(count);
        emit translationStatsChanged(); emit translationLoaded(count);
        m_gameManager->setTranslationMap(m_transEngine->translationMap());
        if (m_gameManager->wsServer()) {
            Protocol::ReloadTransCommand cmd; cmd.translationMap = m_transEngine->translationMap();
            m_gameManager->wsServer()->sendMessage(QString::fromStdString(cmd.toJson().dump()));
        }
    } else {
        m_statusText = "Failed to load translation file"; emit statusTextChanged();
    }
}

void MainWindow::extractText() {
    if (!m_gamePath.isEmpty()) {
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
        m_transStats = QString("%1 strings extracted").arg(list.size());
        emit translationStatsChanged(); emit extractionComplete(list);
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

void MainWindow::onInjectionSucceeded() { m_connected = true; m_statusText = "Connected"; emit statusTextChanged(); emit connectionChanged(); }
void MainWindow::onInjectionFailed(const QString& reason) { m_connected = false; m_statusText = "Failed: " + reason; emit statusTextChanged(); emit connectionChanged(); }
