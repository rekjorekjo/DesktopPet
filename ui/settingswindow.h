#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QListWidget>
#include <QMainWindow>
#include <QStackedWidget>
#include <QPainterPath>

class PetManagePage;
class ActionSettingsPage;
class ApiConfigPage;
class PersonalizationPage;
class AboutPage;

class GlassBackgroundWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GlassBackgroundWidget(QWidget *parent = nullptr);

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
    void petOpacityChanged(double opacity);

private:
    void setupUi();
    void setupSidebar();
    void setupPages();
    void connectSignals();
    void applyTheme();

    GlassBackgroundWidget *m_centralWidget;
    QListWidget *m_sidebar;
    QStackedWidget *m_stackedWidget;

    PetManagePage *m_petManagePage;
    ActionSettingsPage *m_actionSettingsPage;
    ApiConfigPage *m_apiConfigPage;
    PersonalizationPage *m_personalizationPage;
    AboutPage *m_aboutPage;
};

#endif // SETTINGSWINDOW_H
