#ifndef CHATCOMPLETIONSERVICE_H
#define CHATCOMPLETIONSERVICE_H

#include "models/apiconfig.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

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

signals:
    void requestFinished(const QString &requestId, const QString &content);
    void requestFailed(const QString &requestId, const QString &errorMessage);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    static QString generateRequestId();

    QNetworkAccessManager *m_network;
    QHash<QNetworkReply *, QString> m_pendingReplies;
};

#endif // CHATCOMPLETIONSERVICE_H
