#ifndef PERSONALIZATIONPAGE_H
#define PERSONALIZATIONPAGE_H

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QSlider>
#include <QWidget>

class QFrame;

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

private slots:
    void onThemeChanged(int index);
    void onOpacityChanged(int value);
    void onAutoStartOnBootChanged(bool enabled);
    void onAutoPlayOnLaunchChanged(bool enabled);
    void onOpenSettingsOnLaunchChanged(bool enabled);

private:
    void setupUi();
    void connectSignals();
    void applyTheme();

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QLabel *m_titleLabel;

    QFrame *m_appearanceCard;
    QLabel *m_appearanceCardTitle;
    QLabel *m_themeLabel;
    QComboBox *m_themeComboBox;

    QFrame *m_displayCard;
    QLabel *m_displayCardTitle;
    QLabel *m_opacityLabel;
    QSlider *m_opacitySlider;
    QLabel *m_opacityValueLabel;

    QFrame *m_startupCard;
    QLabel *m_startupCardTitle;
    QCheckBox *m_autoStartOnBootCheckBox;
    QCheckBox *m_autoPlayOnLaunchCheckBox;
    QCheckBox *m_openSettingsOnLaunchCheckBox;
};

#endif // PERSONALIZATIONPAGE_H
