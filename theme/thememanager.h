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
    QString listWidgetStyleSheet() const;

signals:
    void themeChanged();

private:
    ThemeManager();
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void loadThemes();
    ThemePalette getDefaultPalette() const;

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