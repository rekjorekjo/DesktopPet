#include "updatemanager.h"

#include "core/appversion.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QtGlobal>

UpdateManager::UpdateManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(nullptr)
{
    m_networkManager = new QNetworkAccessManager(this);
}

UpdateManager::~UpdateManager()
{
}

void UpdateManager::checkForUpdates()
{
    QString apiUrl = QString("https://api.github.com/repos/%1/%2/releases/latest")
                         .arg(QString::fromUtf8(GitHubOwner), QString::fromUtf8(GitHubRepo));

    QUrl url(apiUrl);
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/vnd.github+json");
    request.setRawHeader("User-Agent", "DesktopPet");
    request.setRawHeader("X-GitHub-Api-Version", "2022-11-28");

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
    });
}

void UpdateManager::onReplyFinished(QNetworkReply *reply)
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

    QJsonObject obj = doc.object();
    QString tagName = obj["tag_name"].toString();
    QString htmlUrl = obj["html_url"].toString();

    if (tagName.isEmpty()) {
        qWarning() << "Update check failed: No version info found";
        emit checkFailed(tr("检查更新失败，请检查网络连接或稍后重试。"));
        return;
    }

    QString currentTag = QString::fromUtf8(APP_VERSION_TAG);
    if (tagName != currentTag) {
        emit updateAvailable(tagName, htmlUrl);
    } else {
        emit noUpdateAvailable(tagName);
    }
}

QString UpdateManager::friendlyErrorMessage(const QString &rawError) const
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
