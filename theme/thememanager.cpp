#include "thememanager.h"

#include "core/appsettings.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>
#include <QSet>

namespace {

double clamp01(double value)
{
    return qBound(0.0, value, 1.0);
}

QColor validColor(const QString &value, const QColor &fallback)
{
    QColor color(value);
    return color.isValid() ? color : fallback;
}

QColor mixColor(const QColor &a, const QColor &b, double t)
{
    t = clamp01(t);
    const double inv = 1.0 - t;
    return QColor(
        qBound(0, qRound(a.red() * inv + b.red() * t), 255),
        qBound(0, qRound(a.green() * inv + b.green() * t), 255),
        qBound(0, qRound(a.blue() * inv + b.blue() * t), 255),
        qBound(0, qRound(a.alpha() * inv + b.alpha() * t), 255)
    );
}

QColor lightenColor(const QColor &color, double amount)
{
    return mixColor(color, QColor(255, 255, 255), amount);
}

QColor darkenColor(const QColor &color, double amount)
{
    return mixColor(color, QColor(0, 0, 0), amount);
}

double colorLuma(const QColor &color)
{
    return (0.2126 * color.red() + 0.7152 * color.green() + 0.0722 * color.blue()) / 255.0;
}

QString colorToRgbaString(const QColor &color, int alpha)
{
    return QString("rgba(%1, %2, %3, %4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(qBound(0, alpha, 255));
}

} // namespace

ThemeManager& ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager()
    : m_currentThemeIndex(0)
{
    loadThemes();
    int savedIndex = AppSettings::themeIndex();
    if (savedIndex >= 0 && savedIndex < m_themeIds.size()) {
        m_currentThemeIndex = savedIndex;
    } else if (!m_themeIds.isEmpty()) {
        qWarning() << "ThemeManager: saved theme index out of range"
                   << savedIndex << "theme count" << m_themeIds.size()
                   << ", falling back to 0";
        m_currentThemeIndex = 0;
        AppSettings::setThemeIndex(0);
    }
}

int ThemeManager::extractColorComponent(const QString &hexColor, int index)
{
    QString color = hexColor.trimmed();
    if (color.startsWith('#')) {
        color = color.mid(1);
    }
    
    if (color.length() == 3) {
        color = QString("%1%1%2%2%3%3").arg(color[0], color[1], color[2]);
    }
    
    if (color.length() >= 6) {
        bool ok = false;
        int component = color.mid(index * 2, 2).toInt(&ok, 16);
        return ok ? component : 0;
    }
    
    return 0;
}

QString ThemeManager::colorToRgba(const QString &hexColor, int alpha)
{
    int r = extractColorComponent(hexColor, 0);
    int g = extractColorComponent(hexColor, 1);
    int b = extractColorComponent(hexColor, 2);
    return QString("rgba(%1, %2, %3, %4)").arg(r).arg(g).arg(b).arg(alpha);
}

void ThemeManager::loadThemes()
{
    QFile file(":/themes/themes.json");
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ThemeManager: failed to open :/themes/themes.json:" << file.errorString();
        ThemePalette fallback = getDefaultPalette();
        m_themeIds = QStringList{"light"};
        m_themeNames = QStringList{tr("浅色")};
        m_palettes["light"] = fallback;
        return;
    }
    
    QByteArray jsonData = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "ThemeManager: invalid themes.json at offset"
                   << parseError.offset << parseError.errorString();
        ThemePalette fallback = getDefaultPalette();
        m_themeIds = QStringList{"light"};
        m_themeNames = QStringList{tr("浅色")};
        m_palettes["light"] = fallback;
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray themes = root["themes"].toArray();
    
    if (themes.isEmpty()) {
        qWarning() << "ThemeManager: themes.json does not contain any themes";
        ThemePalette fallback = getDefaultPalette();
        m_themeIds = QStringList{"light"};
        m_themeNames = QStringList{tr("浅色")};
        m_palettes["light"] = fallback;
        return;
    }
    
    m_themeIds.clear();
    m_themeNames.clear();
    m_palettes.clear();
    QSet<QString> seenThemeIds;
    
    for (const QJsonValue &themeVal : themes) {
        if (!themeVal.isObject()) {
            qWarning() << "ThemeManager: skipped non-object theme entry";
            continue;
        }

        QJsonObject themeObj = themeVal.toObject();
        QString id = themeObj["id"].toString();
        QString name = themeObj["name"].toString(id);

        if (id.isEmpty()) {
            qWarning() << "ThemeManager: skipped theme with empty id";
            continue;
        }
        if (seenThemeIds.contains(id)) {
            qWarning() << "ThemeManager: skipped duplicated theme id" << id;
            continue;
        }
        seenThemeIds.insert(id);

        ThemePalette palette;
        palette.pageBackground = themeObj["pageBackground"].toString("#f5f5f5");
        palette.cardBackground = themeObj["cardBackground"].toString("#ffffff");
        palette.border = themeObj["border"].toString("#e0e0e0");
        palette.sidebarBackground = themeObj["sidebarBackground"].toString("#ffffff");
        palette.sidebarText = themeObj["sidebarText"].toString("#374151");
        palette.sidebarHoverBg = themeObj["sidebarHoverBg"].toString("#f3f4f6");
        palette.sidebarHoverText = themeObj["sidebarHoverText"].toString("#1f2937");
        palette.sidebarSelectedBg = themeObj["sidebarSelectedBg"].toString("#eaf3ff");
        palette.sidebarSelectedText = themeObj["sidebarSelectedText"].toString("#2f80ed");
        palette.buttonPrimary = themeObj["buttonPrimary"].toString("#1976d2");
        palette.buttonPrimaryText = themeObj["buttonPrimaryText"].toString("#ffffff");
        palette.buttonHover = themeObj["buttonHover"].toString("#1565c0");
        palette.buttonPressed = themeObj["buttonPressed"].toString("#0d47a1");
        palette.textPrimary = themeObj["textPrimary"].toString("#333333");
        palette.textSecondary = themeObj["textSecondary"].toString("#555555");
        palette.scrollHandle = themeObj["scrollHandle"].toString("#c0c0c0");
        palette.inputBackground = themeObj["inputBackground"].toString("#fafafa");
        palette.secondaryHover = themeObj["secondaryHover"].toString("#e8e8e8");
        palette.dangerBackground = themeObj["dangerBackground"].toString("#fff5f5");
        palette.dangerText = themeObj["dangerText"].toString("#d32f2f");
        palette.dangerBorder = themeObj["dangerBorder"].toString("#ffcdd2");
        palette.dangerHover = themeObj["dangerHover"].toString("#ffebee");
        palette.iconPrimary = themeObj["iconPrimary"].toString("#374151");
        palette.iconOnAccent = themeObj["iconOnAccent"].toString("#ffffff");
        palette.iconMuted = themeObj["iconMuted"].toString("#9ca3af");

        palette.accent = themeObj["accent"].toString(palette.buttonPrimary);
        palette.accentHover = themeObj["accentHover"].toString(palette.buttonHover);
        palette.accentPressed = themeObj["accentPressed"].toString(palette.buttonPressed);
        palette.accentSoft = themeObj["accentSoft"].toString(palette.sidebarSelectedBg);

        palette.cardHoverBackground = themeObj["cardHoverBackground"].toString(palette.secondaryHover);
        palette.cardBorder = themeObj["cardBorder"].toString(palette.border);
        palette.separator = themeObj["separator"].toString(palette.border);

        palette.titleText = themeObj["titleText"].toString(palette.textPrimary);
        palette.subtitleText = themeObj["subtitleText"].toString(palette.textPrimary);
        palette.textMuted = themeObj["textMuted"].toString(palette.textSecondary);
        palette.linkText = themeObj["linkText"].toString(palette.buttonPrimary);
        palette.placeholderText = themeObj["placeholderText"].toString(palette.textSecondary);
        palette.disabledText = themeObj["disabledText"].toString(palette.textSecondary);

        palette.buttonPrimaryHover = themeObj["buttonPrimaryHover"].toString(palette.buttonHover);
        palette.buttonPrimaryPressed = themeObj["buttonPrimaryPressed"].toString(palette.buttonPressed);

        palette.buttonSecondaryBg = themeObj["buttonSecondaryBg"].toString(palette.cardBackground);
        palette.buttonSecondaryText = themeObj["buttonSecondaryText"].toString(palette.textPrimary);
        palette.buttonSecondaryBorder = themeObj["buttonSecondaryBorder"].toString(palette.border);
        palette.buttonSecondaryHover = themeObj["buttonSecondaryHover"].toString(palette.secondaryHover);
        palette.buttonSecondaryPressed = themeObj["buttonSecondaryPressed"].toString(palette.secondaryHover);

        palette.dangerPressed = themeObj["dangerPressed"].toString(palette.dangerBackground);

        palette.inputText = themeObj["inputText"].toString(palette.textPrimary);
        palette.inputBorder = themeObj["inputBorder"].toString(palette.border);
        palette.inputHoverBorder = themeObj["inputHoverBorder"].toString(palette.border);
        palette.inputFocusBorder = themeObj["inputFocusBorder"].toString(palette.buttonPrimary);

        palette.listBackground = themeObj["listBackground"].toString(palette.inputBackground);
        palette.listText = themeObj["listText"].toString(palette.textPrimary);
        palette.listHoverBg = themeObj["listHoverBg"].toString(palette.secondaryHover);
        palette.listSelectedBg = themeObj["listSelectedBg"].toString(palette.sidebarSelectedBg);
        palette.listSelectedText = themeObj["listSelectedText"].toString(palette.sidebarSelectedText);
        palette.listDisabledText = themeObj["listDisabledText"].toString(palette.textSecondary);

        palette.tabText = themeObj["tabText"].toString(palette.textSecondary);
        palette.tabHoverBg = themeObj["tabHoverBg"].toString(palette.secondaryHover);
        palette.tabSelectedBg = themeObj["tabSelectedBg"].toString(palette.cardBackground);
        palette.tabSelectedText = themeObj["tabSelectedText"].toString(palette.textPrimary);
        palette.tabBorder = themeObj["tabBorder"].toString(palette.border);

        palette.menuBackground = themeObj["menuBackground"].toString(palette.cardBackground);
        palette.menuText = themeObj["menuText"].toString(palette.textPrimary);
        palette.menuHoverBg = themeObj["menuHoverBg"].toString(palette.sidebarSelectedBg);
        palette.menuHoverText = themeObj["menuHoverText"].toString(palette.sidebarSelectedText);
        palette.menuSeparator = themeObj["menuSeparator"].toString(palette.border);

        palette.checkboxBorder = themeObj["checkboxBorder"].toString(palette.border);
        palette.checkboxCheckedBg = themeObj["checkboxCheckedBg"].toString(palette.buttonPrimary);
        palette.checkboxCheckColor = themeObj["checkboxCheckColor"].toString(palette.buttonPrimaryText);

        palette.scrollTrack = themeObj["scrollTrack"].toString(palette.pageBackground);
        palette.scrollHandleHover = themeObj["scrollHandleHover"].toString(palette.textSecondary);
        palette.sliderGroove = themeObj["sliderGroove"].toString(palette.border);
        palette.sliderHandle = themeObj["sliderHandle"].toString(palette.buttonPrimary);
        palette.sliderHandleHover = themeObj["sliderHandleHover"].toString(palette.buttonHover);

        palette.success = themeObj["success"].toString("#22c55e");
        palette.warning = themeObj["warning"].toString("#f59e0b");
        palette.error = themeObj["error"].toString("#ef4444");
        palette.info = themeObj["info"].toString("#3b82f6");

        palette.selectionBackground = themeObj["selectionBackground"].toString(palette.sidebarSelectedBg);

        m_themeIds.append(id);
        m_themeNames.append(name);
        m_palettes[id] = palette;
    }
    
    if (m_themeIds.isEmpty()) {
        qWarning() << "ThemeManager: no valid theme entries were loaded";
        ThemePalette fallback = getDefaultPalette();
        m_themeIds = QStringList{"light"};
        m_themeNames = QStringList{tr("浅色")};
        m_palettes["light"] = fallback;
    }
}

ThemePalette ThemeManager::getDefaultPalette() const
{
    ThemePalette p;
    p.pageBackground = "#F5F7FA";
    p.cardBackground = "#FFFFFF";
    p.border = "#E5EAF0";
    p.sidebarBackground = "#FFFFFF";
    p.sidebarText = "#374151";
    p.sidebarHoverBg = "#F3F4F6";
    p.sidebarHoverText = "#1F2937";
    p.sidebarSelectedBg = "#EAF3FF";
    p.sidebarSelectedText = "#2F80ED";
    p.buttonPrimary = "#2F80ED";
    p.buttonPrimaryText = "#FFFFFF";
    p.buttonHover = "#1E6AD4";
    p.buttonPressed = "#1459B3";
    p.textPrimary = "#1F2937";
    p.textSecondary = "#6B7280";
    p.scrollHandle = "#C4CDD5";
    p.inputBackground = "#FFFFFF";
    p.secondaryHover = "#E5E7EB";
    p.dangerBackground = "#FEE2E2";
    p.dangerText = "#DC2626";
    p.dangerBorder = "#FECACA";
    p.dangerHover = "#FECACA";
    p.iconPrimary = "#374151";
    p.iconOnAccent = "#FFFFFF";
    p.iconMuted = "#9CA3AF";

    p.accent = "#2F80ED";
    p.accentHover = "#1E6AD4";
    p.accentPressed = "#1459B3";
    p.accentSoft = "#EAF3FF";

    p.cardHoverBackground = "#F3F4F6";
    p.cardBorder = "#E5EAF0";
    p.separator = "#E5EAF0";

    p.titleText = "#1F2937";
    p.subtitleText = "#374151";
    p.textMuted = "#6B7280";
    p.linkText = "#2F80ED";
    p.placeholderText = "#9CA3AF";
    p.disabledText = "#9CA3AF";

    p.buttonPrimaryHover = "#1E6AD4";
    p.buttonPrimaryPressed = "#1459B3";

    p.buttonSecondaryBg = "#FFFFFF";
    p.buttonSecondaryText = "#1F2937";
    p.buttonSecondaryBorder = "#E5EAF0";
    p.buttonSecondaryHover = "#F3F4F6";
    p.buttonSecondaryPressed = "#E5E7EB";

    p.dangerPressed = "#FCA5A5";

    p.inputText = "#1F2937";
    p.inputBorder = "#E5EAF0";
    p.inputHoverBorder = "#D1D5DB";
    p.inputFocusBorder = "#2F80ED";

    p.listBackground = "#FFFFFF";
    p.listText = "#1F2937";
    p.listHoverBg = "#F3F4F6";
    p.listSelectedBg = "#EAF3FF";
    p.listSelectedText = "#2F80ED";
    p.listDisabledText = "#9CA3AF";

    p.tabText = "#6B7280";
    p.tabHoverBg = "#F3F4F6";
    p.tabSelectedBg = "#FFFFFF";
    p.tabSelectedText = "#1F2937";
    p.tabBorder = "#E5EAF0";

    p.menuBackground = "#FFFFFF";
    p.menuText = "#1F2937";
    p.menuHoverBg = "#EAF3FF";
    p.menuHoverText = "#2F80ED";
    p.menuSeparator = "#E5EAF0";

    p.checkboxBorder = "#D1D5DB";
    p.checkboxCheckedBg = "#2F80ED";
    p.checkboxCheckColor = "#FFFFFF";

    p.scrollTrack = "#F3F4F6";
    p.scrollHandleHover = "#9CA3AF";
    p.sliderGroove = "#E5EAF0";
    p.sliderHandle = "#2F80ED";
    p.sliderHandleHover = "#1E6AD4";

    p.success = "#22C55E";
    p.warning = "#F59E0B";
    p.error = "#EF4444";
    p.info = "#3B82F6";

    p.selectionBackground = "#EAF3FF";

    return p;
}

ThemePalette ThemeManager::currentPalette() const
{
    if (m_currentThemeIndex >= 0 && m_currentThemeIndex < m_themeIds.size()) {
        QString key = m_themeIds.at(m_currentThemeIndex);
        return m_palettes.value(key, getDefaultPalette());
    }
    return getDefaultPalette();
}

int ThemeManager::currentThemeIndex() const
{
    return m_currentThemeIndex;
}

void ThemeManager::setThemeByIndex(int index)
{
    if (index >= 0 && index < m_themeIds.size() && index != m_currentThemeIndex) {
        m_currentThemeIndex = index;
        AppSettings::setThemeIndex(index);
        emit themeChanged();
    }
}

int ThemeManager::themeCount() const
{
    return m_themeNames.size();
}

QString ThemeManager::themeName(int index) const
{
    if (index >= 0 && index < m_themeNames.size()) {
        return m_themeNames.at(index);
    }
    return tr("浅色");
}

QString ThemeManager::themeIdAt(int index) const
{
    if (index >= 0 && index < m_themeIds.size()) {
        return m_themeIds.at(index);
    }
    return QString();
}

QStringList ThemeManager::availableThemes() const
{
    return m_themeNames;
}

QString ThemeManager::pageBackgroundColor() const
{
    return currentPalette().pageBackground;
}

QString ThemeManager::cardBackgroundColor() const
{
    return currentPalette().cardBackground;
}

QString ThemeManager::borderColor() const
{
    return currentPalette().border;
}

QString ThemeManager::sidebarSelectedColor() const
{
    return currentPalette().sidebarSelectedBg;
}

QString ThemeManager::sidebarSelectedTextColor() const
{
    return currentPalette().sidebarSelectedText;
}

QString ThemeManager::buttonPrimaryColor() const
{
    return currentPalette().buttonPrimary;
}

QString ThemeManager::buttonPrimaryTextColor() const
{
    return currentPalette().buttonPrimaryText;
}

QString ThemeManager::textPrimaryColor() const
{
    return currentPalette().textPrimary;
}

QString ThemeManager::textSecondaryColor() const
{
    return currentPalette().textSecondary;
}

SoftCardGradientColors ThemeManager::softCardGradientColors(bool panel, qreal hoverProgress) const
{
    ThemePalette p = currentPalette();

    const QColor base = validColor(p.cardBackground, QColor("#ffffff"));
    const QColor accent = validColor(p.accent, validColor(p.buttonPrimary, base));
    const QColor accentSoft = validColor(p.accentSoft, accent);
    const QColor borderBase = validColor(p.cardBorder, validColor(p.border, accent));

    const double strength = clamp01(AppSettings::cardGradientStrength() / 100.0);
    const double hover = clamp01(hoverProgress);
    const double surfaceScale = panel ? 0.82 : 1.0;
    const double gradientAmount = strength * surfaceScale * (1.0 + 0.18 * hover);
    const bool dark = colorLuma(base) < 0.46;

    const QColor softAccent = mixColor(accent, accentSoft, dark ? 0.25 : 0.32);

    SoftCardGradientColors colors;
    colors.base = base;
    colors.topLeft = mixColor(base, accent, 0.18 * gradientAmount);
    colors.topRight = mixColor(base, softAccent, 0.14 * gradientAmount);
    colors.bottomRight = dark
        ? lightenColor(base, 0.04 * gradientAmount)
        : darkenColor(base, 0.025 * gradientAmount);
    colors.highlight = dark ? lightenColor(base, 0.28) : QColor(255, 255, 255);
    colors.border = mixColor(borderBase, accent, 0.05 + 0.10 * strength + 0.08 * hover);
    colors.shadow = dark ? QColor(0, 0, 0) : darkenColor(borderBase, 0.28);
    colors.baseAlpha = 255;
    colors.highlightAlpha = qBound(0, qRound((dark ? 11 : 16) * strength + 5 * hover), dark ? 18 : 26);
    colors.borderAlpha = qBound(dark ? 125 : 110,
                                qRound((dark ? 140 : 122) + 24 * strength + 32 * hover),
                                dark ? 205 : 190);
    colors.shadowAlpha = qBound(0, qRound((dark ? 22 : 14) + 12 * hover), dark ? 48 : 34);
    return colors;
}

QPixmap ThemeManager::drawArrowIcon(bool up, const QColor &color, int width, int height) const
{
    QPixmap pixmap(width, height);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QPen pen(color, 2.0);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);
    
    QPainterPath path;
    qreal arrowWidth = width * 0.8;
    qreal arrowHeight = height * 0.5;
    qreal centerX = width / 2.0;
    qreal centerY = height / 2.0;
    
    if (up) {
        path.moveTo(centerX - arrowWidth / 2, centerY + arrowHeight / 2);
        path.lineTo(centerX, centerY - arrowHeight / 2);
        path.lineTo(centerX + arrowWidth / 2, centerY + arrowHeight / 2);
    } else {
        path.moveTo(centerX - arrowWidth / 2, centerY - arrowHeight / 2);
        path.lineTo(centerX, centerY + arrowHeight / 2);
        path.lineTo(centerX + arrowWidth / 2, centerY - arrowHeight / 2);
    }
    
    painter.drawPath(path);
    return pixmap;
}

QPixmap ThemeManager::spinUpIcon() const
{
    ThemePalette p = currentPalette();
    return drawArrowIcon(true, QColor(p.iconPrimary), 10, 6);
}

QPixmap ThemeManager::spinDownIcon() const
{
    ThemePalette p = currentPalette();
    return drawArrowIcon(false, QColor(p.iconPrimary), 10, 6);
}

QPixmap ThemeManager::comboDownIcon() const
{
    ThemePalette p = currentPalette();
    return drawArrowIcon(false, QColor(p.iconPrimary), 10, 6);
}

QString ThemeManager::sidebarStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QListWidget {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-right: 1px solid %3;"
        "  outline: none;"
        "}"
        "QListWidget::item {"
        "  padding: 12px 20px;"
        "  border: none;"
        "  color: %2;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: %4;"
        "  color: %5;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: %6;"
        "  color: %7;"
        "}"
    ).arg(p.sidebarBackground, p.sidebarText, p.border, 
          p.sidebarSelectedBg, p.sidebarSelectedText,
          p.sidebarHoverBg, p.sidebarHoverText);
}

QString ThemeManager::cardStyleSheet(const QString &objectName) const
{
    ThemePalette p = currentPalette();
    return QString(
        "QFrame#%1 {"
        "  background-color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 10px;"
        "}"
        "QFrame#%1:hover {"
        "  background-color: %4;"
        "}"
    ).arg(objectName, p.cardBackground, p.cardBorder, p.cardHoverBackground);
}

QString ThemeManager::checkBoxStyleSheet() const
{
    ThemePalette p = currentPalette();

    return QString(
        "QCheckBox {"
        "  color: %1;"
        "  border: none;"
        "  background: transparent;"
        "  spacing: 10px;"
        "}"
        "QCheckBox::indicator {"
        "  width: 20px;"
        "  height: 20px;"
        "  border-radius: 5px;"
        "}"
        "QCheckBox::indicator:unchecked {"
        "  background-color: %2;"
        "  border: 2px solid %3;"
        "}"
        "QCheckBox::indicator:unchecked:hover {"
        "  border-color: %4;"
        "  background-color: %5;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: %4;"
        "  border: 2px solid %4;"
        "  image: url(:/icons/checkmark_white.svg);"
        "}"
        "QCheckBox::indicator:checked:hover {"
        "  background-color: %6;"
        "  border: 2px solid %6;"
        "  image: url(:/icons/checkmark_white.svg);"
        "}"
        "QCheckBox:disabled {"
        "  color: %7;"
        "}"
        "QCheckBox::indicator:disabled {"
        "  background-color: %2;"
        "  border: 2px solid %3;"
        "}"
    )
    .arg(p.textPrimary)           // %1
    .arg(p.inputBackground)       // %2
    .arg(p.checkboxBorder)        // %3
    .arg(p.checkboxCheckedBg)     // %4
    .arg(p.cardBackground)        // %5
    .arg(p.accentHover)           // %6
    .arg(p.disabledText);         // %7
}

QString ThemeManager::primaryButtonStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 20px;"
        "}"
        "QPushButton:hover {"
        "  background-color: %3;"
        "}"
        "QPushButton:pressed {"
        "  background-color: %4;"
        "}"
        "QPushButton:disabled {"
        "  background-color: %5;"
        "  color: %6;"
        "  border: 1px solid %7;"
        "}"
    ).arg(p.buttonPrimary, p.buttonPrimaryText, p.buttonPrimaryHover, p.buttonPrimaryPressed,
          p.buttonSecondaryBg, p.disabledText, p.buttonSecondaryBorder);
}

QString ThemeManager::secondaryButtonStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  padding: 8px 20px;"
        "}"
        "QPushButton:hover {"
        "  background-color: %4;"
        "}"
        "QPushButton:pressed {"
        "  background-color: %5;"
        "}"
        "QPushButton:disabled {"
        "  background-color: %1;"
        "  color: %6;"
        "  border: 1px solid %3;"
        "}"
    ).arg(p.buttonSecondaryBg, p.buttonSecondaryText, p.buttonSecondaryBorder,
          p.buttonSecondaryHover, p.buttonSecondaryPressed, p.disabledText);
}

QString ThemeManager::dangerButtonStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  padding: 8px 20px;"
        "}"
        "QPushButton:hover {"
        "  background-color: %4;"
        "}"
        "QPushButton:pressed {"
        "  background-color: %5;"
        "}"
        "QPushButton:disabled {"
        "  background-color: %1;"
        "  color: %6;"
        "  border: 1px solid %3;"
        "}"
    ).arg(p.dangerBackground, p.dangerText, p.dangerBorder, p.dangerHover,
          p.dangerPressed, p.disabledText);
}

QString ThemeManager::lineEditStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QLineEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  padding: 0 12px;"
        "  placeholder-text-color: %6;"
        "  selection-background-color: %7;"
        "}"
        "QLineEdit:hover {"
        "  border-color: %4;"
        "}"
        "QLineEdit:focus {"
        "  border-color: %5;"
        "}"
    ).arg(p.inputBackground, p.inputText, p.inputBorder, p.inputHoverBorder,
          p.inputFocusBorder, p.placeholderText, p.selectionBackground);
}

QString ThemeManager::scrollAreaStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QScrollArea { background-color: %1; border: none; }"
        "QScrollBar:vertical { width: 10px; background: %2; }"
        "QScrollBar::handle:vertical { background: %3; border-radius: 5px; min-height: 20px; }"
        "QScrollBar::handle:vertical:hover { background: %4; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar:horizontal { height: 10px; background: %2; }"
        "QScrollBar::handle:horizontal { background: %3; border-radius: 5px; min-width: 20px; }"
        "QScrollBar::handle:horizontal:hover { background: %4; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
    ).arg(p.pageBackground, p.scrollTrack, p.scrollHandle, p.scrollHandleHover);
}

QString ThemeManager::pageStyleSheet() const
{
    return QString("background-color: %1;").arg(currentPalette().pageBackground);
}

QString ThemeManager::titleLabelStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString("color: %1; background-color: %2; padding: 28px 28px 0 28px;")
        .arg(p.titleText, p.pageBackground);
}

QString ThemeManager::comboBoxStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QComboBox {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 8px;"
        "  padding: 6px 12px;"
        "  min-width: 120px;"
        "  padding-right: 40px;"
        "}"
        "QComboBox:hover {"
        "  border-color: %4;"
        "}"
        "QComboBox::drop-down {"
        "  subcontrol-origin: padding;"
        "  subcontrol-position: top right;"
        "  width: 30px;"
        "  border: none;"
        "  border-top-right-radius: 8px;"
        "  border-bottom-right-radius: 8px;"
        "  background-color: %5;"
        "}"
        "QComboBox::drop-down:hover {"
        "  background-color: %6;"
        "}"
        "QComboBox::down-arrow {"
        "  image: url(:/icons/combo_down_white.svg);"
        "  width: 10px;"
        "  height: 6px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: %7;"
        "  color: %8;"
        "  border: 1px solid %3;"
        "  selection-background-color: %9;"
        "  selection-color: %10;"
        "  outline: none;"
        "}"
    ).arg(
        p.inputBackground,       // %1
        p.inputText,             // %2
        p.inputBorder,           // %3
        p.inputHoverBorder,      // %4
        p.accent,                // %5
        p.accentHover,           // %6
        p.menuBackground,        // %7
        p.menuText,              // %8
        p.listSelectedBg,        // %9
        p.listSelectedText       // %10
    );
}

QString ThemeManager::spinBoxStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QSpinBox, QDoubleSpinBox {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 8px;"
        "  padding: 6px 10px;"
        "  padding-right: 34px;"
        "}"
        "QSpinBox:hover, QDoubleSpinBox:hover {"
        "  border-color: %4;"
        "}"
        "QSpinBox:focus, QDoubleSpinBox:focus {"
        "  border-color: %5;"
        "}"
        "QSpinBox::up-button, QDoubleSpinBox::up-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: top right;"
        "  width: 28px;"
        "  border: none;"
        "  border-top-right-radius: 8px;"
        "  background-color: %6;"
        "}"
        "QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover {"
        "  background-color: %7;"
        "}"
        "QSpinBox::down-button, QDoubleSpinBox::down-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: bottom right;"
        "  width: 28px;"
        "  border: none;"
        "  border-bottom-right-radius: 8px;"
        "  background-color: %6;"
        "}"
        "QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {"
        "  background-color: %7;"
        "}"
        "QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {"
        "  image: url(:/icons/spin_up_white.svg);"
        "  width: 10px;"
        "  height: 6px;"
        "}"
        "QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {"
        "  image: url(:/icons/spin_down_white.svg);"
        "  width: 10px;"
        "  height: 6px;"
        "}"
    ).arg(
        p.inputBackground,     // %1
        p.inputText,           // %2
        p.inputBorder,         // %3
        p.inputHoverBorder,    // %4
        p.inputFocusBorder,    // %5
        p.accent,              // %6
        p.accentHover          // %7
    );
}

QString ThemeManager::sliderStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QSlider::groove:horizontal {"
        "  background: %1;"
        "  height: 6px;"
        "  border-radius: 3px;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: %2;"
        "  width: 18px;"
        "  height: 18px;"
        "  margin: -6px 0;"
        "  border-radius: 9px;"
        "}"
        "QSlider::handle:horizontal:hover {"
        "  background: %3;"
        "}"
        "QSlider::sub-page:horizontal {"
        "  background: %2;"
        "  border-radius: 3px;"
        "}"
    ).arg(p.sliderGroove, p.sliderHandle, p.sliderHandleHover);
}

QString ThemeManager::listWidgetStyleSheet() const
{
    ThemePalette p = currentPalette();

    return QString(
        "QListWidget {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-top-color: %4;"
        "  border-radius: 12px;"
        "  outline: none;"
        "  padding: 6px;"
        "}"
        "QListWidget::item {"
        "  background-color: transparent;"
        "  border: 1px solid transparent;"
        "  border-radius: 8px;"
        "  margin: 2px;"
        "  padding: 10px 12px;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: %5;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: %6;"
        "  color: %7;"
        "  border-color: %8;"
        "}"
        "QListWidget::item:selected:hover {"
        "  background-color: %9;"
        "}"
        "QListWidget::item:disabled {"
        "  color: %10;"
        "}"
    ).arg(
        p.listBackground,
        p.listText,
        p.inputBorder,
        p.inputBorder,
        p.listHoverBg,
        p.listSelectedBg,
        p.listSelectedText,
        p.inputFocusBorder,
        p.listSelectedBg,
        p.listDisabledText
    );
}

QString ThemeManager::menuStyleSheet() const
{
    ThemePalette p = currentPalette();

    return QString(
        "QMenu {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  padding: 4px;"
        "}"
        "QMenu::item {"
        "  padding: 8px 24px;"
        "  border-radius: 4px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: %4;"
        "  color: %5;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  background-color: %6;"
        "  margin: 4px 8px;"
        "}"
    ).arg(
        p.menuBackground,
        p.menuText,
        p.border,
        p.menuHoverBg,
        p.menuHoverText,
        p.menuSeparator
    );
}

QString ThemeManager::splitterStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QSplitter::handle {"
        "  background-color: %1;"
        "}"
    ).arg(p.border);
}

QString ThemeManager::tabWidgetStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QTabWidget::pane {"
        "  border: 1px solid %1;"
        "  border-radius: 6px;"
        "  background-color: %2;"
        "}"
        "QTabBar::tab {"
        "  background-color: %3;"
        "  color: %4;"
        "  padding: 8px 16px;"
        "  border-top-left-radius: 6px;"
        "  border-top-right-radius: 6px;"
        "  margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "  background-color: %5;"
        "  color: %6;"
        "  border: 1px solid %1;"
        "  border-bottom: none;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background-color: %7;"
        "}"
    ).arg(p.tabBorder, p.cardBackground, p.pageBackground, p.tabText, p.tabSelectedBg, p.tabSelectedText, p.tabHoverBg);
}

QString ThemeManager::dialogStyleSheet() const
{
    ThemePalette p = currentPalette();

    return QString(
        "QDialog {"
        "  background-color: %1;"
        "  color: %2;"
        "}"
        "QDialog QLabel {"
        "  background-color: transparent;"
        "  color: %2;"
        "}"
        "QDialog QFrame {"
        "  background-color: transparent;"
        "}"
        "QToolTip {"
        "  background-color: %3;"
        "  color: %2;"
        "  border: 1px solid %4;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "}"
    ).arg(p.pageBackground, p.textPrimary, p.cardBackground, p.border);
}

QString ThemeManager::timeEditStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QTimeEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "}"
        "QTimeEdit:hover {"
        "  border-color: %4;"
        "}"
        "QTimeEdit:focus {"
        "  border-color: %5;"
        "}"
        "QTimeEdit::up-button, QTimeEdit::down-button {"
        "  background-color: %6;"
        "  border: none;"
        "  width: 16px;"
        "}"
        "QTimeEdit::up-button:hover, QTimeEdit::down-button:hover {"
        "  background-color: %7;"
        "}"
    )
    .arg(p.inputBackground, p.inputText, p.inputBorder, p.inputHoverBorder, p.inputFocusBorder, p.accent, p.accentHover);
}

QString ThemeManager::softCardStyleSheet(int borderRadius, int opacity) const
{
    Q_UNUSED(opacity);
    const SoftCardGradientColors normal = softCardGradientColors(false, 0.0);
    const SoftCardGradientColors hover = softCardGradientColors(false, 1.0);

    return QString(
        "QFrame#softCard {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 %1,"
        "    stop:0.48 %2,"
        "    stop:1 %3);"
        "  border: 1px solid %4;"
        "  border-radius: %5px;"
        "}"
        "QFrame#softCard:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 %6,"
        "    stop:0.48 %7,"
        "    stop:1 %8);"
        "  border-color: %9;"
        "}"
    )
    .arg(colorToRgbaString(normal.topLeft, normal.baseAlpha),
         colorToRgbaString(normal.topRight, normal.baseAlpha),
         colorToRgbaString(normal.bottomRight, normal.baseAlpha),
         colorToRgbaString(normal.border, normal.borderAlpha))
    .arg(borderRadius)
    .arg(colorToRgbaString(hover.topLeft, hover.baseAlpha),
         colorToRgbaString(hover.topRight, hover.baseAlpha),
         colorToRgbaString(hover.bottomRight, hover.baseAlpha),
         colorToRgbaString(hover.border, hover.borderAlpha));
}

QString ThemeManager::softPanelStyleSheet(int borderRadius, int opacity) const
{
    Q_UNUSED(opacity);
    const SoftCardGradientColors normal = softCardGradientColors(true, 0.0);
    const SoftCardGradientColors hover = softCardGradientColors(true, 1.0);

    return QString(
        "QFrame#softPanel {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 %1,"
        "    stop:0.5 %2,"
        "    stop:1 %3);"
        "  border: 1px solid %4;"
        "  border-radius: %5px;"
        "}"
        "QFrame#softPanel:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 %6,"
        "    stop:0.5 %7,"
        "    stop:1 %8);"
        "  border-color: %9;"
        "}"
    )
    .arg(colorToRgbaString(normal.topLeft, normal.baseAlpha),
         colorToRgbaString(normal.topRight, normal.baseAlpha),
         colorToRgbaString(normal.bottomRight, normal.baseAlpha),
         colorToRgbaString(normal.border, normal.borderAlpha))
    .arg(borderRadius)
    .arg(colorToRgbaString(hover.topLeft, hover.baseAlpha),
         colorToRgbaString(hover.topRight, hover.baseAlpha),
         colorToRgbaString(hover.bottomRight, hover.baseAlpha),
         colorToRgbaString(hover.border, hover.borderAlpha));
}

QString ThemeManager::softButtonStyleSheet(int borderRadius, int opacity) const
{
    Q_UNUSED(borderRadius);
    Q_UNUSED(opacity);
    return secondaryButtonStyleSheet();
}

QString ThemeManager::softSecondaryButtonStyleSheet(int borderRadius, int opacity) const
{
    Q_UNUSED(borderRadius);
    Q_UNUSED(opacity);
    return secondaryButtonStyleSheet();
}

QString ThemeManager::softSidebarStyleSheet(int borderRadius, int opacity) const
{
    Q_UNUSED(borderRadius);
    Q_UNUSED(opacity);
    return sidebarStyleSheet();
}

QString ThemeManager::softPageStyleSheet() const
{
    return QString(
        "QWidget#softPageSurface {"
        "  background-color: transparent;"
        "  border: none;"
        "}"
    );
}

QString ThemeManager::softScrollAreaStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QScrollArea { background-color: transparent; border: none; }"
        "QScrollBar:vertical { width: 10px; background: transparent; }"
        "QScrollBar::handle:vertical { background: %1; border-radius: 5px; min-height: 20px; }"
        "QScrollBar::handle:vertical:hover { background: %2; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar:horizontal { height: 10px; background: transparent; }"
        "QScrollBar::handle:horizontal { background: %1; border-radius: 5px; min-width: 20px; }"
        "QScrollBar::handle:horizontal:hover { background: %2; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
    ).arg(p.scrollHandle, p.scrollHandleHover);
}

QString ThemeManager::softTitleLabelStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString("color: %1; background-color: transparent;")
        .arg(p.titleText);
}
