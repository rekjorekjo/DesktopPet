#ifndef DISPLAYSETTINGSPAGE_H
#define DISPLAYSETTINGSPAGE_H

#include <QComboBox>
#include <QLabel>
#include <QScrollArea>
#include <QWidget>

class QFrame;

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
    QFrame *m_appearanceCard;
    QLabel *m_appearanceCardTitle;
    QLabel *m_themeLabel;
    QComboBox *m_themeComboBox;
};

#endif // DISPLAYSETTINGSPAGE_H
