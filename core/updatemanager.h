#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class UpdateManager : public QObject
{
    Q_OBJECT

public:
    explicit UpdateManager(QObject *parent = nullptr);
    ~UpdateManager();

    void checkForUpdates();

signals:
    void updateAvailable(const QString &latestVersion, const QString &releaseUrl);
    void noUpdateAvailable(const QString &latestVersion);
    void checkFailed(const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_networkManager;
};

#endif // UPDATEMANAGER_H
