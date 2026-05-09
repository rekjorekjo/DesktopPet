#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QListWidget>
#include <QMainWindow>
#include <QStackedWidget>
#include <QPainterPath>

class PetManagePage;
class ActionSettingsPage;
class ApiConfigPage;
class LogPage;
class PersonalizationPage;
class AboutPage;
class SettingsTitleBar;

class SoftGradientBackgroundWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SoftGradientBackgroundWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class SidebarContainerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarContainerWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent = nullptr);
    ~SettingsWindow();

signals:
    void startPetRequested();
    void pausePetRequested();
    void applyPetConfigRequested();
    void actionConfigApplied();
    void petOpacityChanged(double opacity);
    void baseMoveSpeedChanged(int speed);

protected:
    void changeEvent(QEvent *event) override;

private:
    void setupUi();
    void setupSidebar();
    void setupPages();
    void connectSignals();
    void applyTheme();

    SoftGradientBackgroundWidget *m_centralWidget;
    SettingsTitleBar *m_titleBar;
    SidebarContainerWidget *m_sidebarContainer;
    QListWidget *m_sidebar;
    QStackedWidget *m_stackedWidget;

    PetManagePage *m_petManagePage;
    ActionSettingsPage *m_actionSettingsPage;
    ApiConfigPage *m_apiConfigPage;
    LogPage *m_logPage;
    PersonalizationPage *m_personalizationPage;
    AboutPage *m_aboutPage;
};

#endif // SETTINGSWINDOW_H
