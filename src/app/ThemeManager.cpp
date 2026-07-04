#include "app/ThemeManager.h"
#include <QDebug>

namespace {
struct Variant { QString name; QVariantMap colors; QVariantMap metrics; };
struct LayoutDef { QString label; QStringList variants; };

const QMap<QString, Variant>& variantMap() {
    static QMap<QString, Variant> m;
    if (m.isEmpty()) {
        // ===== Mtool-Classic =====
        m["Mtool:Cyan"] = {"Cyan",
            {{"bg","#E3F2FD"},{"surface","#FFFFFF"},{"surfaceAlt","#BBDEFB"},{"inputBg","#FFFFFF"},
             {"border","#90CAF9"},{"text","#0D47A1"},{"textMuted","#546E7A"},
             {"accent","#1976D2"},{"accentText","#FFFFFF"},{"accentSoft","#42A5F5"},
             {"danger","#D32F2F"},{"success","#2E7D32"},{"warning","#F57C00"},
             {"chipBg","#E1F5FE"},{"chipText","#01579B"},{"chipHover","#B3E5FC"},
             {"dropBg","#E1F5FE"},{"dropBorder","#0288D1"}},
            {{"radius",14.0},{"radiusSmall",8.0},{"radiusPill",16.0},{"borderWidth",1.0},
             {"chipHeight",28.0},{"dropHeight",120.0},{"spacing",8.0},{"padding",12.0}}};

        m["Mtool:Slate"] = {"Slate",
            {{"bg","#ECEFF1"},{"surface","#FFFFFF"},{"surfaceAlt","#CFD8DC"},{"inputBg","#FFFFFF"},
             {"border","#90A4AE"},{"text","#263238"},{"textMuted","#546E7A"},
             {"accent","#455A64"},{"accentText","#FFFFFF"},{"accentSoft","#607D8B"},
             {"danger","#C62828"},{"success","#2E7D32"},{"warning","#EF6C00"},
             {"chipBg","#CFD8DC"},{"chipText","#263238"},{"chipHover","#B0BEC5"},
             {"dropBg","#CFD8DC"},{"dropBorder","#455A64"}},
            {{"radius",14.0},{"radiusSmall",8.0},{"radiusPill",16.0},{"borderWidth",1.0},
             {"chipHeight",28.0},{"dropHeight",120.0},{"spacing",8.0},{"padding",12.0}}};

        m["Mtool:Forest"] = {"Forest",
            {{"bg","#E8F5E9"},{"surface","#FFFFFF"},{"surfaceAlt","#C8E6C9"},{"inputBg","#FFFFFF"},
             {"border","#81C784"},{"text","#1B5E20"},{"textMuted","#558B2F"},
             {"accent","#2E7D32"},{"accentText","#FFFFFF"},{"accentSoft","#43A047"},
             {"danger","#C62828"},{"success","#1B5E20"},{"warning","#E65100"},
             {"chipBg","#DCEDC8"},{"chipText","#1B5E20"},{"chipHover","#C5E1A5"},
             {"dropBg","#DCEDC8"},{"dropBorder","#388E3C"}},
            {{"radius",14.0},{"radiusSmall",8.0},{"radiusPill",16.0},{"borderWidth",1.0},
             {"chipHeight",28.0},{"dropHeight",120.0},{"spacing",8.0},{"padding",12.0}}};

        // ===== Side-Drawer =====
        m["Drawer:Light"] = {"Light",
            {{"bg","#FAFAFA"},{"surface","#FFFFFF"},{"surfaceAlt","#F5F5F5"},{"inputBg","#FFFFFF"},
             {"border","#E5E7EB"},{"text","#1F2937"},{"textMuted","#6B7280"},
             {"accent","#10B981"},{"accentText","#FFFFFF"},{"accentSoft","#34D399"},
             {"danger","#EF4444"},{"success","#10B981"},{"warning","#F59E0B"},
             {"chipBg","#ECFDF5"},{"chipText","#047857"},{"chipHover","#D1FAE5"},
             {"dropBg","#F0FDF4"},{"dropBorder","#10B981"}},
            {{"radius",10.0},{"radiusSmall",6.0},{"radiusPill",8.0},{"borderWidth",1.0},
             {"chipHeight",36.0},{"dropHeight",100.0},{"spacing",12.0},{"padding",16.0}}};

        m["Drawer:Dark"] = {"Dark",
            {{"bg","#0F172A"},{"surface","#1E293B"},{"surfaceAlt","#334155"},{"inputBg","#1E293B"},
             {"border","#475569"},{"text","#F8FAFC"},{"textMuted","#94A3B8"},
             {"accent","#10B981"},{"accentText","#0F172A"},{"accentSoft","#34D399"},
             {"danger","#F87171"},{"success","#34D399"},{"warning","#FBBF24"},
             {"chipBg","#064E3B"},{"chipText","#6EE7B7"},{"chipHover","#065F46"},
             {"dropBg","#134E4A"},{"dropBorder","#10B981"}},
            {{"radius",10.0},{"radiusSmall",6.0},{"radiusPill",8.0},{"borderWidth",1.0},
             {"chipHeight",36.0},{"dropHeight",100.0},{"spacing",12.0},{"padding",16.0}}};

        // ===== Floating-Panels =====
        m["Floating:Dark"] = {"Dark",
            {{"bg","#1E1E2E"},{"surface","#313244"},{"surfaceAlt","#45475A"},{"inputBg","#45475A"},
             {"border","#6C7086"},{"text","#CDD6F4"},{"textMuted","#A6ADC8"},
             {"accent","#CBA6F7"},{"accentText","#1E1E2E"},{"accentSoft","#A78BFA"},
             {"danger","#F38BA8"},{"success","#A6E3A1"},{"warning","#F9E2AF"},
             {"chipBg","#45475A"},{"chipText","#CDD6F4"},{"chipHover","#585B70"},
             {"dropBg","#313244"},{"dropBorder","#CBA6F7"}},
            {{"radius",12.0},{"radiusSmall",6.0},{"radiusPill",10.0},{"borderWidth",1.0},
             {"chipHeight",32.0},{"dropHeight",80.0},{"spacing",10.0},{"padding",14.0}}};

        m["Floating:Dawn"] = {"Dawn",
            {{"bg","#FFF7ED"},{"surface","#FFFFFF"},{"surfaceAlt","#FFE4C7"},{"inputBg","#FFFFFF"},
             {"border","#FDBA74"},{"text","#7C2D12"},{"textMuted","#9A3412"},
             {"accent","#EA580C"},{"accentText","#FFFFFF"},{"accentSoft","#FB923C"},
             {"danger","#DC2626"},{"success","#16A34A"},{"warning","#D97706"},
             {"chipBg","#FFEDD5"},{"chipText","#9A3412"},{"chipHover","#FED7AA"},
             {"dropBg","#FFEDD5"},{"dropBorder","#EA580C"}},
            {{"radius",12.0},{"radiusSmall",6.0},{"radiusPill",10.0},{"borderWidth",1.0},
             {"chipHeight",32.0},{"dropHeight",80.0},{"spacing",10.0},{"padding",14.0}}};
    }
    return m;
}

const QMap<QString, LayoutDef>& layoutMap() {
    static QMap<QString, LayoutDef> m;
    if (m.isEmpty()) {
        m["Mtool"]    = {"Mtool-Classic (\u60dc\u60dc\u00B7\u7C7B\u4F20\u7EDF)", {"Cyan", "Slate", "Forest"}};
        m["Drawer"]   = {"Side-Drawer (\u4FA7\u8FB9\u5BFC\u822A)",               {"Light", "Dark"}};
        m["Floating"] = {"Floating-Panels (\u6D6E\u52A8\u9762\u677F)",           {"Dark", "Dawn"}};
    }
    return m;
}

} // namespace

ThemeManager::ThemeManager(QObject* parent)
    : QObject(parent), m_settings("RTranslator", "Theme")
{
    QString saved = m_settings.value("theme", "Mtool:Cyan").toString();
    if (!variantMap().contains(saved)) saved = "Mtool:Cyan";
    applyTheme(saved);
}

QString ThemeManager::currentTheme() const { return m_currentTheme; }
QString ThemeManager::layoutName() const {
    int i = m_currentTheme.indexOf(':');
    return i < 0 ? m_currentTheme : m_currentTheme.left(i);
}
QString ThemeManager::variantName() const {
    int i = m_currentTheme.indexOf(':');
    return i < 0 ? QString() : m_currentTheme.mid(i + 1);
}
QVariantMap ThemeManager::colors() const { return m_colors; }
QVariantMap ThemeManager::metrics() const { return m_metrics; }

QStringList ThemeManager::themes() const {
    QStringList out;
    out << variantMap().keys();
    return out;
}
QStringList ThemeManager::layouts() const {
    QStringList out;
    out << layoutMap().keys();
    return out;
}

QStringList ThemeManager::variantsOf(const QString& layout) const {
    return layoutMap().value(layout).variants;
}

void ThemeManager::setCurrentTheme(const QString& theme) {
    if (theme == m_currentTheme) return;
    if (!variantMap().contains(theme)) return;
    applyTheme(theme);
    saveTheme();
}

void ThemeManager::setLayout(const QString& layout) {
    if (!layoutMap().contains(layout)) return;
    QStringList varList = layoutMap().value(layout).variants;
    QString v = variantName();
    if (v.isEmpty() || !varList.contains(v)) v = varList.first();
    QString key = layout + ":" + v;
    if (key == m_currentTheme) return;
    applyTheme(key);
    saveTheme();
}

void ThemeManager::setVariant(const QString& variant) {
    QString l = layoutName();
    QString key = l + ":" + variant;
    if (!variantMap().contains(key)) return;
    applyTheme(key);
    saveTheme();
}

void ThemeManager::cycleTheme() {
    QStringList all = themes();
    int idx = all.indexOf(m_currentTheme);
    if (idx < 0) idx = -1;
    QString next = all[(idx + 1) % all.size()];
    applyTheme(next);
    saveTheme();
}

void ThemeManager::applyTheme(const QString& key) {
    m_currentTheme = key;
    const Variant& v = variantMap().value(key);
    m_colors = v.colors;
    m_metrics = v.metrics;
    emit colorsChanged();
    emit metricsChanged();
    emit currentThemeChanged();
}

void ThemeManager::saveTheme() {
    m_settings.setValue("theme", m_currentTheme);
    m_settings.sync();
}
