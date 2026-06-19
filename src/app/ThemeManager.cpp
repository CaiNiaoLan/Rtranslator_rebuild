#include "app/ThemeManager.h"

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent), m_settings("RTranslator", "Theme")
{
    QString saved = m_settings.value("theme", "Aurora").toString();
    loadTheme(saved);
}

QString ThemeManager::currentTheme() const { return m_currentTheme; }

void ThemeManager::setCurrentTheme(const QString& theme) {
    if (theme == m_currentTheme) return;
    loadTheme(theme);
    saveTheme();
}

QStringList ThemeManager::themeNames() const {
    return {"Glassmorphism", "Brutalism", "Bento", "Minimalism", "Aurora"};
}

void ThemeManager::cycleTheme() {
    QStringList names = themeNames();
    int idx = names.indexOf(m_currentTheme);
    if (idx < 0) idx = names.size() - 1;
    setCurrentTheme(names[(idx + 1) % names.size()]);
}

void ThemeManager::saveTheme() {
    m_settings.setValue("theme", m_currentTheme);
    m_settings.sync();
}

QVariantMap ThemeManager::colors() const { return m_colors; }
QVariantMap ThemeManager::metrics() const { return m_metrics; }

void ThemeManager::loadTheme(const QString& name) {
    m_currentTheme = name;
    m_colors.clear();
    m_metrics.clear();

    if (name == "Glassmorphism") {
        m_colors["bg"] = "#1a1a2e";
        m_colors["surface"] = "#30ffffff";
        m_colors["surfaceAlt"] = "#20c8d0e0";
        m_colors["border"] = "#30ffffff";
        m_colors["text"] = "#e8e8f0";
        m_colors["textMuted"] = "#8890a8";
        m_colors["accent"] = "#7890e8";
        m_colors["accentText"] = "#ffffff";
        m_colors["danger"] = "#f06070";
        m_colors["success"] = "#60c888";
        m_colors["warning"] = "#e8b850";
        m_colors["inputBg"] = "#20ffffff";
        m_metrics["radius"] = 16;
        m_metrics["radiusSmall"] = 10;
        m_metrics["borderWidth"] = 1;
    }
    else if (name == "Brutalism") {
        m_colors["bg"] = "#ffffff";
        m_colors["surface"] = "#ffffff";
        m_colors["surfaceAlt"] = "#f0f000";
        m_colors["border"] = "#000000";
        m_colors["text"] = "#000000";
        m_colors["textMuted"] = "#444444";
        m_colors["accent"] = "#0000ff";
        m_colors["accentText"] = "#ffffff";
        m_colors["danger"] = "#ff0000";
        m_colors["success"] = "#00ff00";
        m_colors["warning"] = "#ffff00";
        m_colors["inputBg"] = "#ffffff";
        m_metrics["radius"] = 0;
        m_metrics["radiusSmall"] = 0;
        m_metrics["borderWidth"] = 3;
    }
    else if (name == "Bento") {
        m_colors["bg"] = "#f2efea";
        m_colors["surface"] = "#ffffff";
        m_colors["surfaceAlt"] = "#faf7f2";
        m_colors["border"] = "#e8e2d8";
        m_colors["text"] = "#2c2416";
        m_colors["textMuted"] = "#8a8070";
        m_colors["accent"] = "#e8a87c";
        m_colors["accentText"] = "#ffffff";
        m_colors["danger"] = "#d4695a";
        m_colors["success"] = "#7ba587";
        m_colors["warning"] = "#d4a853";
        m_colors["inputBg"] = "#faf7f2";
        m_metrics["radius"] = 24;
        m_metrics["radiusSmall"] = 16;
        m_metrics["borderWidth"] = 1;
    }
    else if (name == "Minimalism") {
        m_colors["bg"] = "#ffffff";
        m_colors["surface"] = "#fafafa";
        m_colors["surfaceAlt"] = "#f5f5f5";
        m_colors["border"] = "#e5e5e5";
        m_colors["text"] = "#111111";
        m_colors["textMuted"] = "#888888";
        m_colors["accent"] = "#111111";
        m_colors["accentText"] = "#ffffff";
        m_colors["danger"] = "#cc3333";
        m_colors["success"] = "#338833";
        m_colors["warning"] = "#cc8833";
        m_colors["inputBg"] = "#ffffff";
        m_metrics["radius"] = 2;
        m_metrics["radiusSmall"] = 1;
        m_metrics["borderWidth"] = 1;
    }
    else if (name == "Aurora") {
        m_colors["bg"] = "#0d0d1a";
        m_colors["surface"] = "#151530";
        m_colors["surfaceAlt"] = "#1a1a3e";
        m_colors["border"] = "#3030a0";
        m_colors["text"] = "#d0d0f0";
        m_colors["textMuted"] = "#6868b0";
        m_colors["accent"] = "#8870ff";
        m_colors["accentText"] = "#ffffff";
        m_colors["danger"] = "#ff5570";
        m_colors["success"] = "#50e890";
        m_colors["warning"] = "#ffb050";
        m_colors["inputBg"] = "#121228";
        m_metrics["radius"] = 12;
        m_metrics["radiusSmall"] = 8;
        m_metrics["borderWidth"] = 1;
    }

    emit colorsChanged();
    emit metricsChanged();
    emit currentThemeChanged();
}
