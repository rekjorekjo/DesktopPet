#include "updatemanager.h"

#include "appversion.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

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
        emit checkFailed(reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit checkFailed(tr("解析响应失败"));
        return;
    }

    if (!doc.isObject()) {
        emit checkFailed(tr("响应格式错误"));
        return;
    }

    QJsonObject obj = doc.object();
    QString tagName = obj["tag_name"].toString();
    QString htmlUrl = obj["html_url"].toString();

    if (tagName.isEmpty()) {
        emit checkFailed(tr("未找到版本信息"));
        return;
    }

    QString currentTag = QString::fromUtf8(APP_VERSION_TAG);
    if (tagName != currentTag) {
        emit updateAvailable(tagName, htmlUrl);
    } else {
        emit noUpdateAvailable(tagName);
    }
}
