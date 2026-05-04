#include "thememanager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPainterPath>

ThemeManager& ThemeManager::instance()
{
    static ThemeManager manager;
    return manager;
}

ThemeManager::ThemeManager()
    : m_currentThemeIndex(0)
{
    loadThemes();
}

void ThemeManager::loadThemes()
{
    QFile file(":/themes/themes.json");
    
    if (!file.open(QIODevice::ReadOnly)) {
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
        ThemePalette fallback = getDefaultPalette();
        m_themeIds = QStringList{"light"};
        m_themeNames = QStringList{tr("浅色")};
        m_palettes["light"] = fallback;
        return;
    }
    
    QJsonObject root = doc.object();
    QJsonArray themes = root["themes"].toArray();
    
    if (themes.isEmpty()) {
        ThemePalette fallback = getDefaultPalette();
        m_themeIds = QStringList{"light"};
        m_themeNames = QStringList{tr("浅色")};
        m_palettes["light"] = fallback;
        return;
    }
    
    m_themeIds.clear();
    m_themeNames.clear();
    m_palettes.clear();
    
    for (const QJsonValue &themeVal : themes) {
        QJsonObject themeObj = themeVal.toObject();
        QString id = themeObj["id"].toString();
        QString name = themeObj["name"].toString(id);
        
        if (id.isEmpty()) continue;
        
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
        
        m_themeIds.append(id);
        m_themeNames.append(name);
        m_palettes[id] = palette;
    }
    
    if (m_themeIds.isEmpty()) {
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
    ).arg(objectName, p.cardBackground, p.border);
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
    )
    .arg(p.textPrimary)         // %1
    .arg(p.inputBackground)     // %2
    .arg(p.iconMuted)           // %3
    .arg(p.buttonPrimary)       // %4
    .arg(p.cardBackground)      // %5
    .arg(p.buttonHover);        // %6
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
    ).arg(p.buttonPrimary, p.buttonPrimaryText, p.buttonHover, p.buttonPressed);
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
    ).arg(p.cardBackground, p.textPrimary, p.border, p.secondaryHover);
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
    ).arg(p.dangerBackground, p.dangerText, p.dangerBorder, p.dangerHover);
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
        "}"
        "QLineEdit:focus {"
        "  border-color: %4;"
        "}"
    ).arg(p.inputBackground, p.textPrimary, p.border, p.buttonPrimary);
}

QString ThemeManager::scrollAreaStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QScrollArea { background-color: %1; border: none; }"
        "QScrollBar:vertical { width: 10px; }"
        "QScrollBar::handle:vertical { background: %2; border-radius: 5px; min-height: 20px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
    ).arg(p.pageBackground, p.scrollHandle);
}

QString ThemeManager::pageStyleSheet() const
{
    return QString("background-color: %1;").arg(currentPalette().pageBackground);
}

QString ThemeManager::titleLabelStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString("color: %1; background-color: %2; padding: 28px 28px 0 28px;")
        .arg(p.textPrimary, p.pageBackground);
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
        "  background-color: %4;"
        "}"
        "QComboBox::drop-down:hover {"
        "  background-color: %5;"
        "}"
        "QComboBox::down-arrow {"
        "  image: url(:/icons/combo_down_white.svg);"
        "  width: 10px;"
        "  height: 6px;"
        "}"
        "QComboBox QAbstractItemView {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  selection-background-color: %6;"
        "  selection-color: %7;"
        "  outline: none;"
        "}"
    ).arg(
        p.cardBackground,        // %1
        p.textPrimary,           // %2
        p.border,                // %3
        p.buttonPrimary,         // %4
        p.buttonHover,           // %5
        p.sidebarSelectedBg,     // %6
        p.sidebarSelectedText    // %7
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
        "QSpinBox::up-button, QDoubleSpinBox::up-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: top right;"
        "  width: 28px;"
        "  border: none;"
        "  border-top-right-radius: 8px;"
        "  background-color: %4;"
        "}"
        "QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover {"
        "  background-color: %5;"
        "}"
        "QSpinBox::down-button, QDoubleSpinBox::down-button {"
        "  subcontrol-origin: border;"
        "  subcontrol-position: bottom right;"
        "  width: 28px;"
        "  border: none;"
        "  border-bottom-right-radius: 8px;"
        "  background-color: %4;"
        "}"
        "QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {"
        "  background-color: %5;"
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
        p.cardBackground,    // %1
        p.textPrimary,       // %2
        p.border,            // %3
        p.buttonPrimary,     // %4
        p.buttonHover        // %5
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
    ).arg(p.border, p.buttonPrimary, p.buttonHover);
}

QString ThemeManager::listWidgetStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QListWidget {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 6px;"
        "  outline: none;"
        "}"
        "QListWidget::item {"
        "  padding: 10px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: %4;"
        "  color: %5;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: %6;"
        "}"
    ).arg(p.inputBackground, p.textPrimary, p.border, p.sidebarSelectedBg, p.sidebarSelectedText, p.pageBackground);
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
        "  background-color: %2;"
        "  color: %5;"
        "  border: 1px solid %1;"
        "  border-bottom: none;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background-color: %6;"
        "}"
    ).arg(p.border, p.inputBackground, p.pageBackground, p.textSecondary, p.textPrimary, p.secondaryHover);
}

QString ThemeManager::dialogStyleSheet() const
{
    ThemePalette p = currentPalette();
    return QString(
        "QDialog {"
        "  background-color: %1;"
        "}"
        "QLabel {"
        "  background-color: transparent;"
        "}"
    ).arg(p.pageBackground);
}
