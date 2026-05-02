#ifndef DISPLAYSETTINGSPAGE_H
#define DISPLAYSETTINGSPAGE_H

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QScrollArea>
#include <QWidget>

class DisplaySettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit DisplaySettingsPage(QWidget *parent = nullptr);
    ~DisplaySettingsPage();

public slots:
    void refreshTheme();

private slots:
    void onThemeChanged(int index);

private:
    void setupUi();
    void connectSignals();
    void applyTheme();

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QLabel *m_titleLabel;
    QFrame *m_petCard;
    QFrame *m_movementCard;
    QFrame *m_appearanceCard;
    QFrame *m_systemCard;
    QLabel *m_petCardTitle;
    QLabel *m_movementCardTitle;
    QLabel *m_appearanceCardTitle;
    QLabel *m_systemCardTitle;
    QLabel *m_scaleLabel;
    QLabel *m_speedLabel;
    QLabel *m_themeLabel;
    QDoubleSpinBox *m_scaleSpinBox;
    QSpinBox *m_speedSpinBox;
    QCheckBox *m_autoStartCheckBox;
    QCheckBox *m_alwaysOnTopCheckBox;
    QComboBox *m_themeComboBox;
};

#endif // DISPLAYSETTINGSPAGE_H
