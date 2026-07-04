#pragma once
#include <QObject>
#include <QMap>
#include <QVariantMap>
#include <QSettings>

class ThemeManager : public QObject {
    Q_OBJECT
    // Current theme is a single string in form "layout:variant" e.g. "Mtool:Cyan", "Drawer:Light"
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(QString layoutName READ layoutName NOTIFY currentThemeChanged)
    Q_PROPERTY(QString variantName READ variantName NOTIFY currentThemeChanged)
    Q_PROPERTY(QVariantMap colors READ colors NOTIFY colorsChanged)
    Q_PROPERTY(QVariantMap metrics READ metrics NOTIFY metricsChanged)
    Q_PROPERTY(QStringList themes READ themes CONSTANT)
    Q_PROPERTY(QStringList layouts READ layouts CONSTANT)
public:
    explicit ThemeManager(QObject* parent = nullptr);

    QString currentTheme() const;
    void setCurrentTheme(const QString& theme);
    QString layoutName() const;
    QString variantName() const;

    QVariantMap colors() const;
    QVariantMap metrics() const;
    QStringList themes() const;        // list of "layout:variant"
    QStringList layouts() const;       // list of layouts

    Q_INVOKABLE void cycleTheme();
    Q_INVOKABLE void setLayout(const QString& layout);
    Q_INVOKABLE void setVariant(const QString& variant);
    Q_INVOKABLE QStringList variantsOf(const QString& layout) const;

signals:
    void currentThemeChanged();
    void colorsChanged();
    void metricsChanged();

private:
    void applyTheme(const QString& themeKey);
    void saveTheme();
    QStringList variantsForLayout(const QString& layout) const;

    QString m_currentTheme;   // "layout:variant"
    QVariantMap m_colors;
    QVariantMap m_metrics;
    QSettings m_settings;
};
