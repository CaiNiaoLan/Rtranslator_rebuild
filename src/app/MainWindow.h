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
    Q_PROPERTY(QString currentMapName READ currentMapName NOTIFY currentMapNameChanged)
    Q_PROPERTY(QString currentMapId READ currentMapId NOTIFY currentMapIdChanged)
    Q_PROPERTY(QString playerX READ playerX NOTIFY playerXChanged)
    Q_PROPERTY(QString playerY READ playerY NOTIFY playerYChanged)
    Q_PROPERTY(QString selectedEventName READ selectedEventName NOTIFY selectedEventNameChanged)
    Q_PROPERTY(QString selectedEventDetail READ selectedEventDetail NOTIFY selectedEventDetailChanged)
    Q_PROPERTY(int selectedEventId READ selectedEventId NOTIFY selectedEventIdChanged)
    Q_PROPERTY(QVariantList mapListModel READ mapListModel NOTIFY mapListModelChanged)
    Q_PROPERTY(QVariantList switchListModel READ switchListModel NOTIFY switchListModelChanged)
    Q_PROPERTY(QVariantList variableListModel READ variableListModel NOTIFY variableListModelChanged)
    Q_PROPERTY(QVariantList commonEventListModel READ commonEventListModel NOTIFY commonEventListModelChanged)
    Q_PROPERTY(QVariantList mapEventListModel READ mapEventListModel NOTIFY mapEventListModelChanged)
    Q_PROPERTY(QVariantList partyStatusModel READ partyStatusModel NOTIFY partyStatusModelChanged)
    Q_PROPERTY(QVariantMap currentMapInfo READ currentMapInfo NOTIFY currentMapInfoChanged)
public:
    explicit MainWindow(QQmlApplicationEngine* engine, QObject* parent = nullptr);
    QString gamePath() const;
    void setGamePath(const QString& path);
    bool isConnected() const;
    QString statusText() const;
    QString translationStats() const;
    QString currentMapName() const; QString currentMapId() const;
    QString playerX() const; QString playerY() const;
    QString selectedEventName() const; QString selectedEventDetail() const;
    int selectedEventId() const;
    QVariantList mapListModel() const; QVariantList switchListModel() const;
    QVariantList variableListModel() const; QVariantList commonEventListModel() const;
    QVariantList mapEventListModel() const; QVariantList partyStatusModel() const;
    QVariantMap currentMapInfo() const;
    Q_INVOKABLE void launchGame();
    Q_INVOKABLE void detachGame();
    Q_INVOKABLE void loadGameDir(const QString& dir);
    Q_INVOKABLE void loadTranslationFile(const QString& filePath);
    Q_INVOKABLE void extractText();
    Q_INVOKABLE void exportOriginalText();
    Q_INVOKABLE void readGameState(const QString& path);
    Q_INVOKABLE void writeGameState(const QString& path, const QVariant& value);
    Q_INVOKABLE void triggerBattleVictory();
    Q_INVOKABLE void addItemToInventory(int id, const QString& type, int count);
    Q_INVOKABLE QString browseGameFile();
    Q_INVOKABLE QString browseTranslationFile();
    Q_INVOKABLE void cheatAction(const QString& action);
    // Map tab
    Q_INVOKABLE void teleportToMap(int mapId);
    // Variable tab
    Q_INVOKABLE void toggleSwitch(int id, bool on);
    Q_INVOKABLE void setVariable(int id, int value);
    // Event tab
    Q_INVOKABLE void selectCommonEvent(int id);
    Q_INVOKABLE void selectMapEvent(int id);
    Q_INVOKABLE void triggerSelectedEvent();
signals:
    void gamePathChanged();
    void connectionChanged();
    void statusTextChanged();
    void translationStatsChanged();
    void gameStarted();
    void gameStopped();
    void extractionComplete(const QVariantList& entries);
    void translationLoaded(int count);
    // Tab data signals
    void currentMapNameChanged(); void currentMapIdChanged();
    void playerXChanged(); void playerYChanged();
    void selectedEventNameChanged(); void selectedEventDetailChanged();
    void selectedEventIdChanged();
    void mapListModelChanged(); void switchListModelChanged();
    void variableListModelChanged(); void commonEventListModelChanged();
    void mapEventListModelChanged(); void partyStatusModelChanged();
    void currentMapInfoChanged();
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
    std::vector<TranslationEntry> m_entries;
    // Tab data state
    QString m_currentMapName; QString m_currentMapId;
    QString m_playerX; QString m_playerY;
    QString m_selectedEventName; QString m_selectedEventDetail;
    int m_selectedEventId = 0;
    QVariantList m_mapListModel; QVariantList m_switchListModel;
    QVariantList m_variableListModel; QVariantList m_commonEventListModel;
    QVariantList m_mapEventListModel; QVariantList m_partyStatusModel;
    QVariantMap m_currentMapInfo;
};
