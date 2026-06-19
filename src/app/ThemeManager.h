#pragma once
#include <QObject>
#include <QMap>
#include <QVariantMap>
#include <QSettings>

class ThemeManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(QVariantMap colors READ colors NOTIFY colorsChanged)
    Q_PROPERTY(QVariantMap metrics READ metrics NOTIFY metricsChanged)
public:
    enum Theme { Glassmorphism, Brutalism, Bento, Minimalism, Aurora };
    Q_ENUM(Theme)

    explicit ThemeManager(QObject* parent = nullptr);

    QString currentTheme() const;
    void setCurrentTheme(const QString& theme);

    QVariantMap colors() const;
    QVariantMap metrics() const;

    Q_INVOKABLE void cycleTheme();
    Q_INVOKABLE QStringList themeNames() const;

signals:
    void currentThemeChanged();
    void colorsChanged();
    void metricsChanged();

private:
    void loadTheme(const QString& name);
    void saveTheme();

    QString m_currentTheme;
    QVariantMap m_colors;
    QVariantMap m_metrics;
    QSettings m_settings;
};
