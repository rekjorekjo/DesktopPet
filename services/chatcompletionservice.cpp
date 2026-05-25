#include "chatcompletionservice.h"

#include "services/secretstorageservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUuid>

ChatCompletionService::ChatCompletionService(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    connect(m_network, &QNetworkAccessManager::finished,
            this, &ChatCompletionService::onReplyFinished);
}

ChatCompletionService::~ChatCompletionService() {}

QString ChatCompletionService::sendChatCompletion(const ApiConfig &config,
                                                  const QList<Message> &messages)
{
    const QString requestId = generateRequestId();

    auto failLater = [this, requestId](const QString &message) {
        QTimer::singleShot(0, this, [this, requestId, message]() {
            emit requestFailed(requestId, message);
        });
        return requestId;
    };

    if (messages.isEmpty()) {
        return failLater(tr("消息列表为空。"));
    }

    if (config.apiKey.trimmed().isEmpty()) {
        return failLater(tr("API Key 未配置。"));
    }

    if (config.baseUrl.trimmed().isEmpty()) {
        return failLater(tr("Base URL 未配置。"));
    }

    if (config.model.trimmed().isEmpty()) {
        return failLater(tr("模型名称未配置。"));
    }

    int maxTokens = config.maxTokens;
    if (maxTokens <= 0) maxTokens = 1024;
    double temperature = config.temperature;
    if (temperature < 0.0 || temperature > 2.0) temperature = 0.7;

    // Build URL
    QString baseUrl = config.baseUrl.trimmed();
    while (baseUrl.endsWith('/')) baseUrl.chop(1);
    QUrl url(baseUrl + "/chat/completions");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         ("Bearer " + config.apiKey.trimmed()).toUtf8());

    // Build request body
    QJsonArray messagesArray;
    for (const Message &msg : messages) {
        QJsonObject msgObj;
        msgObj["role"] = msg.role;
        msgObj["content"] = msg.content;
        messagesArray.append(msgObj);
    }

    QJsonObject body;
    body["model"] = config.model.trimmed();
    body["messages"] = messagesArray;
    body["max_tokens"] = maxTokens;
    body["temperature"] = temperature;

    QJsonDocument doc(body);
    QNetworkReply *reply = m_network->post(request, doc.toJson(QJsonDocument::Compact));
    m_pendingReplies.insert(reply, requestId);

    return requestId;
}

void ChatCompletionService::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    QString requestId = m_pendingReplies.take(reply);
    if (requestId.isEmpty()) return;

    if (reply->error() != QNetworkReply::NoError) {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString body = QString::fromUtf8(reply->readAll());

        QString errorMsg;
        if (status > 0) {
            errorMsg = tr("HTTP %1: %2").arg(status)
                         .arg(SecretStorageService::redactSecrets(body.left(200)));
        } else {
            errorMsg = SecretStorageService::redactSecrets(reply->errorString());
        }

        emit requestFailed(requestId, errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit requestFailed(requestId,
                           tr("JSON 解析失败：%1").arg(parseError.errorString()));
        return;
    }

    if (!doc.isObject()) {
        emit requestFailed(requestId, tr("响应格式错误：非 JSON 对象。"));
        return;
    }

    QJsonObject root = doc.object();

    // Check for API error
    if (root.contains("error")) {
        QJsonObject errorObj = root["error"].toObject();
        QString errorMsg = SecretStorageService::redactSecrets(errorObj["message"].toString());
        if (errorMsg.isEmpty()) errorMsg = tr("未知 API 错误。");
        emit requestFailed(requestId, errorMsg);
        return;
    }

    QJsonArray choices = root["choices"].toArray();
    if (choices.isEmpty()) {
        emit requestFailed(requestId, tr("响应中没有 choices。"));
        return;
    }

    QJsonObject firstChoice = choices[0].toObject();
    QJsonObject messageObj = firstChoice["message"].toObject();
    QString content = messageObj["content"].toString();

    if (content.isEmpty()) {
        emit requestFailed(requestId, tr("响应内容为空。"));
        return;
    }

    emit requestFinished(requestId, content);
}

QString ChatCompletionService::generateRequestId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(12);
}
