#ifndef WEBSEARCHSERVICE_H
#define WEBSEARCHSERVICE_H

#include "models/websearchconfig.h"

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

struct WebSearchResult
{
    QString title;
    QString url;
    QString snippet;
    QString source;
    QString publishedAt;
};

class WebSearchService : public QObject
{
    Q_OBJECT

public:
    explicit WebSearchService(QObject *parent = nullptr);
    ~WebSearchService();

    QString search(const QString &query, const WebSearchConfig &config);
    void cancelRequest(const QString &requestId);
    void cancelAllRequests();

signals:
    void searchFinished(const QString &requestId,
                        const QString &query,
                        const QList<WebSearchResult> &results);
    void searchFailed(const QString &requestId,
                      const QString &query,
                      const QString &message);
    void searchCanceled(const QString &requestId);

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onRequestTimeout(QNetworkReply *reply);

private:
    struct PendingSearch
    {
        QString requestId;
        QString query;
        WebSearchProvider provider;
        QTimer *timeoutTimer = nullptr;
        bool canceled = false;
    };

    static QString generateRequestId();
    QString normalizeNetworkError(QNetworkReply *reply, const QByteArray &body);
    QString extractProviderErrorMessage(const QJsonObject &root, WebSearchProvider provider);
    QString statusHint(int statusCode);

    QList<WebSearchResult> parseTavilyResponse(const QJsonObject &root);
    QList<WebSearchResult> parseBraveResponse(const QJsonObject &root);
    QList<WebSearchResult> parseExaResponse(const QJsonObject &root);

    QNetworkAccessManager *m_network;
    QHash<QNetworkReply *, PendingSearch> m_pendingReplies;
    QHash<QString, QNetworkReply *> m_repliesById;
};

#endif // WEBSEARCHSERVICE_H
