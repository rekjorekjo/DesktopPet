#ifndef PERSONALIZATIONPAGE_H
#define PERSONALIZATIONPAGE_H

#include <QCheckBox>
#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QWidget>

#include "widgets/softcardwidget.h"

class QPushButton;
class WheelGuardListWidget;
class QListWidgetItem;

class PersonalizationPage : public QWidget
{
    Q_OBJECT

public:
    explicit PersonalizationPage(QWidget *parent = nullptr);
    ~PersonalizationPage();

public slots:
    void refreshTheme();

signals:
    void petOpacityChanged(double opacity);
    void cardGradientStrengthChanged(int value);
    void randomCardGradientChanged(bool enabled);
    void baseMoveSpeedChanged(int speed);

private slots:
    void onThemeItemClicked(QListWidgetItem *item);
    void onOpacityChanged(int value);
    void onCardGradientStrengthChanged(int value);
    void onRandomCardGradientChanged(bool enabled);
    void onBaseMoveSpeedChanged(int value);
    void onAutoStartOnBootChanged(bool enabled);
    void onAutoPlayOnLaunchChanged(bool enabled);
    void onOpenSettingsOnLaunchChanged(bool enabled);
    void onClearRegistryClicked();

private:
    void setupUi();
    void connectSignals();
    void applyTheme();
    void populateThemeLists();
    bool isLightTheme(const QString &themeId) const;

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QLabel *m_titleLabel;

    SoftCardWidget *m_appearanceCard;
    QLabel *m_appearanceCardTitle;
    QLabel *m_lightThemeLabel;
    WheelGuardListWidget *m_lightThemeList;
    QLabel *m_darkThemeLabel;
    WheelGuardListWidget *m_darkThemeList;

    SoftCardWidget *m_displayCard;
    QLabel *m_displayCardTitle;
    QLabel *m_opacityLabel;
    QSlider *m_opacitySlider;
    QLabel *m_opacityValueLabel;

    QLabel *m_gradientStrengthLabel;
    QSlider *m_gradientStrengthSlider;
    QLabel *m_gradientStrengthValueLabel;
    QCheckBox *m_randomGradientCheckBox;

    QLabel *m_baseMoveSpeedLabel;
    QSlider *m_baseMoveSpeedSlider;
    QLabel *m_baseMoveSpeedValueLabel;

    SoftCardWidget *m_startupCard;
    QLabel *m_startupCardTitle;
    QCheckBox *m_autoStartOnBootCheckBox;
    QCheckBox *m_autoPlayOnLaunchCheckBox;
    QCheckBox *m_openSettingsOnLaunchCheckBox;
    QPushButton *m_clearRegistryButton;
};

#endif // PERSONALIZATIONPAGE_H
