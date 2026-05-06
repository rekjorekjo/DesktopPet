#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QStringList>
#include <QColor>

struct ThemePalette
{
    QString pageBackground;
    QString cardBackground;
    QString border;

    QString sidebarBackground;
    QString sidebarText;
    QString sidebarHoverBg;
    QString sidebarHoverText;
    QString sidebarSelectedBg;
    QString sidebarSelectedText;

    QString buttonPrimary;
    QString buttonPrimaryText;
    QString buttonHover;
    QString buttonPressed;

    QString textPrimary;
    QString textSecondary;
    QString scrollHandle;
    QString inputBackground;
    QString secondaryHover;

    QString dangerBackground;
    QString dangerText;
    QString dangerBorder;
    QString dangerHover;

    QString iconPrimary;
    QString iconOnAccent;
    QString iconMuted;

    QString accent;
    QString accentHover;
    QString accentPressed;
    QString accentSoft;

    QString cardHoverBackground;
    QString cardBorder;
    QString separator;

    QString titleText;
    QString subtitleText;
    QString textMuted;
    QString linkText;
    QString placeholderText;
    QString disabledText;

    QString buttonPrimaryHover;
    QString buttonPrimaryPressed;

    QString buttonSecondaryBg;
    QString buttonSecondaryText;
    QString buttonSecondaryBorder;
    QString buttonSecondaryHover;
    QString buttonSecondaryPressed;

    QString dangerPressed;

    QString inputText;
    QString inputBorder;
    QString inputHoverBorder;
    QString inputFocusBorder;

    QString listBackground;
    QString listText;
    QString listHoverBg;
    QString listSelectedBg;
    QString listSelectedText;
    QString listDisabledText;

    QString tabText;
    QString tabHoverBg;
    QString tabSelectedBg;
    QString tabSelectedText;
    QString tabBorder;

    QString menuBackground;
    QString menuText;
    QString menuHoverBg;
    QString menuHoverText;
    QString menuSeparator;

    QString checkboxBorder;
    QString checkboxCheckedBg;
    QString checkboxCheckColor;

    QString scrollTrack;
    QString scrollHandleHover;
    QString sliderGroove;
    QString sliderHandle;
    QString sliderHandleHover;

    QString success;
    QString warning;
    QString error;
    QString info;

    QString selectionBackground;

    QString glassBackground;
    QString glassBackgroundStrong;
    QString glassBorder;
    QString glassHighlight;
    QString glassShadow;
    QString glassTextPrimary;
    QString glassTextSecondary;
};

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager& instance();

    int currentThemeIndex() const;
    void setThemeByIndex(int index);
    int themeCount() const;
    QString themeName(int index) const;
    QString themeIdAt(int index) const;
    QStringList availableThemes() const;

    ThemePalette currentPalette() const;

    // 旧接口先保留，避免各页面继续报错
    QString pageBackgroundColor() const;
    QString cardBackgroundColor() const;
    QString borderColor() const;
    QString sidebarSelectedColor() const;
    QString sidebarSelectedTextColor() const;
    QString buttonPrimaryColor() const;
    QString buttonPrimaryTextColor() const;
    QString textPrimaryColor() const;
    QString textSecondaryColor() const;

    // 动态箭头/保留的图标接口
    QPixmap spinUpIcon() const;
    QPixmap spinDownIcon() const;
    QPixmap comboDownIcon() const;
    QPixmap checkboxUncheckedIcon() const;
    QPixmap checkboxCheckedIcon() const;

    // 样式表接口
    QString sidebarStyleSheet() const;
    QString cardStyleSheet(const QString &objectName) const;
    QString checkBoxStyleSheet() const;
    QString primaryButtonStyleSheet() const;
    QString secondaryButtonStyleSheet() const;
    QString dangerButtonStyleSheet() const;
    QString lineEditStyleSheet() const;
    QString scrollAreaStyleSheet() const;
    QString pageStyleSheet() const;
    QString titleLabelStyleSheet() const;
    QString comboBoxStyleSheet() const;
    QString spinBoxStyleSheet() const;
    QString sliderStyleSheet() const;
    QString listWidgetStyleSheet() const;
    QString menuStyleSheet() const;
    QString splitterStyleSheet() const;
    QString tabWidgetStyleSheet() const;
    QString dialogStyleSheet() const;
    QString timeEditStyleSheet() const;

    QString glassCardStyleSheet(int borderRadius = 12, int opacity = 40) const;
    QString glassPanelStyleSheet(int borderRadius = 8, int opacity = 30) const;
    QString glassButtonStyleSheet(int borderRadius = 6, int opacity = 35) const;
    QString glassSidebarStyleSheet(int borderRadius = 8, int opacity = 30) const;
    QString glassPageStyleSheet() const;
    QString glassScrollAreaStyleSheet() const;
    QString glassTitleLabelStyleSheet() const;

signals:
    void themeChanged();

private:
    ThemeManager();
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void loadThemes();
    ThemePalette getDefaultPalette() const;

    static int extractColorComponent(const QString &hexColor, int index);
    static QString colorToRgba(const QString &hexColor, int alpha);

    QPixmap drawArrowIcon(bool up, const QColor &color, int width, int height) const;
    QPixmap drawCheckboxIcon(bool checked, const QColor &bgColor,
                             const QColor &borderColor, const QColor &checkColor) const;

private:
    int m_currentThemeIndex;
    QMap<QString, ThemePalette> m_palettes;
    QStringList m_themeIds;
    QStringList m_themeNames;
};

#endif // THEMEMANAGER_H