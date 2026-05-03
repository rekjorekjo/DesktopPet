#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QListWidget>
#include <QMainWindow>
#include <QStackedWidget>

class PetManagePage;
class ActionSettingsPage;
class ApiConfigPage;
class DisplaySettingsPage;
class AboutPage;

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

signals:
    void startPetRequested();
    void pausePetRequested();

private:
    void setupUi();
    void setupSidebar();
    void setupPages();
    void connectSignals();
    void applyTheme();

    QWidget *m_centralWidget;
    QListWidget *m_sidebar;
    QStackedWidget *m_stackedWidget;

    PetManagePage *m_petManagePage;
    ActionSettingsPage *m_actionSettingsPage;
    ApiConfigPage *m_apiConfigPage;
    DisplaySettingsPage *m_displaySettingsPage;
    AboutPage *m_aboutPage;
};

#endif // SETTINGSWINDOW_H
