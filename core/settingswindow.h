#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QListWidget>
#include <QMainWindow>
#include <QStackedWidget>

class DefaultPetPage;
class ApiConfigPage;
class DisplaySettingsPage;
class AboutPage;

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

private:
    void setupUi();
    void setupSidebar();
    void setupPages();
    void connectSignals();
    void applyTheme();

    QWidget *m_centralWidget;
    QListWidget *m_sidebar;
    QStackedWidget *m_stackedWidget;

    DefaultPetPage *m_defaultPetPage;
    ApiConfigPage *m_apiConfigPage;
    DisplaySettingsPage *m_displaySettingsPage;
    AboutPage *m_aboutPage;
};

#endif // SETTINGSWINDOW_H
