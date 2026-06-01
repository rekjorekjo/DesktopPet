#ifndef UPDATESERVICE_H
#define UPDATESERVICE_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

struct UpdateInfo {
    bool valid = false;
    bool updateAvailable = false;
    QString currentVersion;
    QString latestVersion;
    QString releaseName;
    QString releaseNotes;
    QString htmlUrl;
    QString assetName;
    QString downloadUrl;
    QString sha256;
    qint64 assetSize = 0;
};

class UpdateService : public QObject
{
    Q_OBJECT

public:
    explicit UpdateService(QObject *parent = nullptr);
    ~UpdateService();

    void checkForUpdates();
    void downloadUpdate(const UpdateInfo &info);

    // Parse a GitHub release JSON object into UpdateInfo.
    // Static so it can be tested without network.
    static UpdateInfo parseReleaseJson(const QJsonObject &obj, const QString &currentVersion);

    // Parse a manifest JSON object (latest.json) into UpdateInfo.
    // Static so it can be tested without network.
    static UpdateInfo parseManifestJson(const QJsonObject &obj, const QString &currentVersion);

signals:
    void checkFinished(const UpdateInfo &info);
    void checkFailed(const QString &message);
    void downloadProgress(qint64 received, qint64 total);
    void downloadFinished(const QString &filePath);
    void downloadFailed(const QString &message);

private slots:
    void onManifestReplyFinished(QNetworkReply *reply);
    void onCheckReplyFinished(QNetworkReply *reply);
    void onDownloadReplyFinished(QNetworkReply *reply);
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    QString friendlyErrorMessage(const QString &rawError) const;
    bool verifySha256(const QString &filePath, const QString &expectedHash) const;

    QNetworkAccessManager *m_networkManager;
    UpdateInfo m_pendingDownloadInfo;
};

#endif // UPDATESERVICE_H
