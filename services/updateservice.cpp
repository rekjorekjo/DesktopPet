#include "updateservice.h"

#include "core/appversion.h"
#include "core/versionutils.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QUrl>

UpdateService::UpdateService(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

UpdateService::~UpdateService()
{
}

void UpdateService::checkForUpdates()
{
    QString apiUrl = QString("https://api.github.com/repos/%1/%2/releases/latest")
                         .arg(QString::fromUtf8(GitHubOwner), QString::fromUtf8(GitHubRepo));

    QUrl url(apiUrl);
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("User-Agent", "DesktopPet-Updater");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onCheckReplyFinished(reply);
    });
}

void UpdateService::onCheckReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString rawError = reply->errorString();
        qWarning() << "Update check failed:" << rawError;
        emit checkFailed(friendlyErrorMessage(rawError));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Update check failed: JSON parse error";
        emit checkFailed(tr("检查更新失败，请检查网络连接或稍后重试。"));
        return;
    }

    if (!doc.isObject()) {
        qWarning() << "Update check failed: Invalid response format";
        emit checkFailed(tr("检查更新失败，请检查网络连接或稍后重试。"));
        return;
    }

    QString currentVersion = QString::fromUtf8(APP_VERSION_TAG);
    UpdateInfo info = parseReleaseJson(doc.object(), currentVersion);

    if (!info.valid) {
        emit checkFailed(tr("检查更新失败，请检查网络连接或稍后重试。"));
        return;
    }

    emit checkFinished(info);
}

UpdateInfo UpdateService::parseReleaseJson(const QJsonObject &obj, const QString &currentVersion)
{
    UpdateInfo info;
    info.currentVersion = currentVersion;

    QString tagName = obj["tag_name"].toString();
    if (tagName.isEmpty()) {
        return info;
    }

    bool draft = obj["draft"].toBool(false);
    bool prerelease = obj["prerelease"].toBool(false);
    if (draft || prerelease) {
        return info;
    }

    info.valid = true;
    info.latestVersion = tagName;
    info.releaseName = obj["name"].toString();
    info.releaseNotes = obj["body"].toString();
    info.htmlUrl = obj["html_url"].toString();

    // Check if update is available
    int cmp = VersionUtils::compareVersions(tagName, currentVersion);
    info.updateAvailable = (cmp > 0);

    if (!info.updateAvailable) {
        return info;
    }

    // Find the Inno Setup installer asset
    QJsonArray assets = obj["assets"].toArray();
    for (const QJsonValue &assetVal : assets) {
        QJsonObject asset = assetVal.toObject();
        QString name = asset["name"].toString();

        // Match DesktopPet_Setup_v*.exe or DesktopPet_Setup*.exe
        if (name.startsWith("DesktopPet_Setup", Qt::CaseInsensitive)
            && name.endsWith(".exe", Qt::CaseInsensitive)) {
            info.assetName = name;
            info.downloadUrl = asset["browser_download_url"].toString();
            info.assetSize = asset["size"].toVariant().toLongLong();

            // Parse SHA-256 digest if available
            QString digest = asset["digest"].toString();
            if (digest.startsWith("sha256:", Qt::CaseInsensitive)) {
                info.sha256 = digest.mid(7).trimmed();
            }

            break;
        }
    }

    return info;
}

void UpdateService::downloadUpdate(const UpdateInfo &info)
{
    if (info.downloadUrl.isEmpty()) {
        emit downloadFailed(tr("没有可用的下载链接。"));
        return;
    }

    m_pendingDownloadInfo = info;

    // Prepare download directory
    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                          + "/DesktopPet/updates";
    QDir dir(downloadDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filePath = downloadDir + "/" + info.assetName;

    // Remove old file if exists
    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }

    QUrl url(info.downloadUrl);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "DesktopPet-Updater");

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, filePath]() {
        onDownloadReplyFinished(reply);
    });
    connect(reply, &QNetworkReply::downloadProgress, this, &UpdateService::onDownloadProgress);
}

void UpdateService::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(bytesReceived, bytesTotal);
}

void UpdateService::onDownloadReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString rawError = reply->errorString();
        qWarning() << "Download failed:" << rawError;
        emit downloadFailed(tr("下载失败，请检查网络连接或稍后重试。"));
        return;
    }

    // Read all data and write to file
    QByteArray data = reply->readAll();

    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                          + "/DesktopPet/updates";
    QString filePath = downloadDir + "/" + m_pendingDownloadInfo.assetName;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit downloadFailed(tr("无法写入下载文件。"));
        return;
    }
    file.write(data);
    file.close();

    // Verify SHA-256 if available
    if (!m_pendingDownloadInfo.sha256.isEmpty()) {
        if (!verifySha256(filePath, m_pendingDownloadInfo.sha256)) {
            QFile::remove(filePath);
            emit downloadFailed(tr("文件校验失败，下载的文件可能已损坏。"));
            return;
        }
    }

    emit downloadFinished(filePath);
}

bool UpdateService::verifySha256(const QString &filePath, const QString &expectedHash) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&file)) {
        return false;
    }

    QString computedHash = QString::fromUtf8(hash.result().toHex());
    return computedHash.compare(expectedHash, Qt::CaseInsensitive) == 0;
}

QString UpdateService::friendlyErrorMessage(const QString &rawError) const
{
    if (rawError.contains("SSL", Qt::CaseInsensitive)
        || rawError.contains("TLS", Qt::CaseInsensitive)
        || rawError.contains("libssl", Qt::CaseInsensitive)
        || rawError.contains("CRYPTO_", Qt::CaseInsensitive)
        || rawError.contains("openssl", Qt::CaseInsensitive)) {
        return tr("检查更新失败，当前环境的 SSL 组件不可用，请检查运行环境。");
    }

    return tr("检查更新失败，请检查网络连接或稍后重试。");
}
