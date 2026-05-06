#ifndef TRAYMANAGER_H
#define TRAYMANAGER_H

#include <QObject>

class QMenu;
class QSystemTrayIcon;

class TrayManager : public QObject
{
    Q_OBJECT

public:
    explicit TrayManager(QObject *parent = nullptr);
    ~TrayManager();

    void show();

signals:
    void showPetRequested();
    void hidePetRequested();
    void startPetRequested();
    void pausePetRequested();
    void openSettingsRequested();
    void quitRequested();
    void emotionRequested(const QString &emotion);

private slots:
    void onTrayActivated();

private:
    void createMenu();

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_menu;
};

#endif // TRAYMANAGER_H
