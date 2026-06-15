#pragma once
#include <QObject>
#include <QQmlApplicationEngine>
#include <QVariantList>
#include <memory>
#include "core/GameManager.h"
#include "core/TranslationEngine.h"
#include "core/CheatController.h"
#include "adapter/RpgMVAdapter.h"

class MainWindow : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString gamePath READ gamePath WRITE setGamePath NOTIFY gamePathChanged)
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectionChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QString translationStats READ translationStats NOTIFY translationStatsChanged)
public:
    explicit MainWindow(QQmlApplicationEngine* engine, QObject* parent = nullptr);
    QString gamePath() const;
    void setGamePath(const QString& path);
    bool isConnected() const;
    QString statusText() const;
    QString translationStats() const;
    Q_INVOKABLE void launchGame();
    Q_INVOKABLE void detachGame();
    Q_INVOKABLE void loadGameDir(const QString& dir);
    Q_INVOKABLE void loadTranslationFile(const QString& filePath);
    Q_INVOKABLE void extractText();
    Q_INVOKABLE void readGameState(const QString& path);
    Q_INVOKABLE void writeGameState(const QString& path, const QVariant& value);
    Q_INVOKABLE void triggerBattleVictory();
    Q_INVOKABLE void addItemToInventory(int id, const QString& type, int count);
signals:
    void gamePathChanged();
    void connectionChanged();
    void statusTextChanged();
    void translationStatsChanged();
    void gameStarted();
    void gameStopped();
    void extractionComplete(const QVariantList& entries);
    void translationLoaded(int count);
private slots:
    void onInjectionSucceeded();
    void onInjectionFailed(const QString& reason);
private:
    QQmlApplicationEngine* m_engine;
    std::unique_ptr<TranslationEngine> m_transEngine;
    std::unique_ptr<GameManager> m_gameManager;
    std::unique_ptr<CheatController> m_cheatController;
    QString m_gamePath;
    QString m_statusText;
    QString m_transStats;
    bool m_connected;
};
