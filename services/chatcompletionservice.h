#ifndef CHATCOMPLETIONSERVICE_H
#define CHATCOMPLETIONSERVICE_H

#include "models/apiconfig.h"

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

class ChatCompletionService : public QObject
{
    Q_OBJECT

public:
    struct Message
    {
        QString role;
        QString content;
    };

    explicit ChatCompletionService(QObject *parent = nullptr);
    ~ChatCompletionService();

    QString sendChatCompletion(const ApiConfig &config,
                               const QList<Message> &messages);

    QString testConnection(const ApiConfig &config);

    void cancelRequest(const QString &requestId);
    void cancelAllRequests();

signals:
    void requestFinished(const QString &requestId, const QString &content);
    void requestFailed(const QString &requestId, const QString &errorMessage);
    void requestCanceled(const QString &requestId);
    void connectionTestFinished(const QString &requestId, bool ok, const QString &message);

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onRequestTimeout(QNetworkReply *reply);

private:
    struct PendingRequest
    {
        QString requestId;
        QString purpose; // "chat" or "connectionTest"
        QTimer *timeoutTimer = nullptr;
        bool canceled = false;
    };

    static QString generateRequestId();
    QString normalizeNetworkError(QNetworkReply *reply, const QByteArray &body);
    QString extractApiErrorMessage(const QJsonObject &root);
    QString statusHint(int statusCode);

    QNetworkAccessManager *m_network;
    QHash<QNetworkReply *, PendingRequest> m_pendingReplies;
    QHash<QString, QNetworkReply *> m_repliesById;
};

#endif // CHATCOMPLETIONSERVICE_H
