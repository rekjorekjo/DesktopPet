#include "chatcompletionservice.h"

#include "models/apiconfig.h"
#include "services/secretstorageservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUuid>

static constexpr int kChatTimeoutMs = 60000;
static constexpr int kConnectionTestTimeoutMs = 20000;

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

    if (config.apiFormat != ApiFormat::OpenAICompatible) {
        return failLater(tr("当前仅支持 OpenAI 兼容格式的 API。"));
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

    if (messages.isEmpty()) {
        return failLater(tr("消息列表为空。"));
    }

    int maxTokens = clampMaxTokens(config.maxTokens);
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

    // Setup timeout
    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, reply]() {
        onRequestTimeout(reply);
    });

    PendingRequest pr;
    pr.requestId = requestId;
    pr.purpose = "chat";
    pr.timeoutTimer = timer;
    pr.canceled = false;
    m_pendingReplies.insert(reply, pr);
    m_repliesById.insert(requestId, reply);

    timer->start(kChatTimeoutMs);

    return requestId;
}

QString ChatCompletionService::testConnection(const ApiConfig &config)
{
    const QString requestId = generateRequestId();

    auto failLater = [this, requestId](const QString &message) {
        QTimer::singleShot(0, this, [this, requestId, message]() {
            emit connectionTestFinished(requestId, false, message);
        });
        return requestId;
    };

    if (config.apiFormat != ApiFormat::OpenAICompatible) {
        return failLater(tr("当前仅支持 OpenAI 兼容格式的 API。"));
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

    // Build URL
    QString baseUrl = config.baseUrl.trimmed();
    while (baseUrl.endsWith('/')) baseUrl.chop(1);
    QUrl url(baseUrl + "/chat/completions");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         ("Bearer " + config.apiKey.trimmed()).toUtf8());

    // Minimal request body
    QJsonObject msgObj;
    msgObj["role"] = "user";
    msgObj["content"] = "ping";

    QJsonArray messagesArray;
    messagesArray.append(msgObj);

    QJsonObject body;
    body["model"] = config.model.trimmed();
    body["messages"] = messagesArray;
    body["max_tokens"] = 8;
    body["temperature"] = 0;

    QJsonDocument doc(body);
    QNetworkReply *reply = m_network->post(request, doc.toJson(QJsonDocument::Compact));

    // Setup timeout
    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, reply]() {
        onRequestTimeout(reply);
    });

    PendingRequest pr;
    pr.requestId = requestId;
    pr.purpose = "connectionTest";
    pr.timeoutTimer = timer;
    pr.canceled = false;
    m_pendingReplies.insert(reply, pr);
    m_repliesById.insert(requestId, reply);

    timer->start(kConnectionTestTimeoutMs);

    return requestId;
}

void ChatCompletionService::cancelRequest(const QString &requestId)
{
    QNetworkReply *reply = m_repliesById.value(requestId);
    if (!reply) return;

    auto it = m_pendingReplies.find(reply);
    if (it == m_pendingReplies.end()) return;

    QTimer *timer = it->timeoutTimer;

    // Remove from hashes first so onReplyFinished sees it as unknown
    m_pendingReplies.erase(it);
    m_repliesById.remove(requestId);

    if (timer) {
        timer->stop();
        timer->deleteLater();
    }

    reply->abort();
    reply->deleteLater();

    emit requestCanceled(requestId);
}

void ChatCompletionService::cancelAllRequests()
{
    const QStringList ids = m_repliesById.keys();
    for (const QString &id : ids) {
        cancelRequest(id);
    }
}

void ChatCompletionService::onRequestTimeout(QNetworkReply *reply)
{
    auto it = m_pendingReplies.find(reply);
    if (it == m_pendingReplies.end()) return;

    QString requestId = it->requestId;
    QString purpose = it->purpose;
    QTimer *timer = it->timeoutTimer;

    // Remove from hashes first so onReplyFinished sees it as unknown
    m_pendingReplies.erase(it);
    m_repliesById.remove(requestId);

    if (timer) {
        timer->stop();
        timer->deleteLater();
    }

    reply->abort();
    reply->deleteLater();

    QString timeoutMsg = (purpose == "connectionTest")
        ? tr("连接测试超时，请检查网络、Base URL 或模型服务状态。")
        : tr("请求超时，请检查网络、Base URL 或模型服务状态。");

    if (purpose == "connectionTest") {
        emit connectionTestFinished(requestId, false, timeoutMsg);
    } else {
        emit requestFailed(requestId, timeoutMsg);
    }
}

void ChatCompletionService::onReplyFinished(QNetworkReply *reply)
{
    auto it = m_pendingReplies.find(reply);
    if (it == m_pendingReplies.end()) {
        reply->deleteLater();
        return;
    }

    PendingRequest pr = *it;
    m_pendingReplies.erase(it);
    m_repliesById.remove(pr.requestId);

    if (pr.timeoutTimer) {
        pr.timeoutTimer->stop();
        pr.timeoutTimer->deleteLater();
    }

    reply->deleteLater();

    // If canceled, ignore the result
    if (pr.canceled) return;

    QString requestId = pr.requestId;
    bool isTest = (pr.purpose == "connectionTest");

    if (reply->error() != QNetworkReply::NoError) {
        // Handle abort (cancel) gracefully - already handled by canceled flag
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            if (isTest) {
                emit connectionTestFinished(requestId, false,
                    tr("请求超时，请检查网络、Base URL 或模型服务状态。"));
            } else {
                emit requestFailed(requestId,
                    tr("请求超时，请检查网络、Base URL 或模型服务状态。"));
            }
            return;
        }

        QByteArray body = reply->readAll();
        QString errorMsg = normalizeNetworkError(reply, body);

        if (isTest) {
            emit connectionTestFinished(requestId, false, errorMsg);
        } else {
            emit requestFailed(requestId, errorMsg);
        }
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QString errMsg = tr("JSON 解析失败：%1").arg(parseError.errorString());
        if (isTest) {
            emit connectionTestFinished(requestId, false, errMsg);
        } else {
            emit requestFailed(requestId, errMsg);
        }
        return;
    }

    if (!doc.isObject()) {
        QString errMsg = tr("响应格式错误：非 JSON 对象。");
        if (isTest) {
            emit connectionTestFinished(requestId, false, errMsg);
        } else {
            emit requestFailed(requestId, errMsg);
        }
        return;
    }

    QJsonObject root = doc.object();

    // Check for API error
    if (root.contains("error")) {
        QString errMsg = extractApiErrorMessage(root);
        if (isTest) {
            emit connectionTestFinished(requestId, false, errMsg);
        } else {
            emit requestFailed(requestId, errMsg);
        }
        return;
    }

    QJsonArray choices = root["choices"].toArray();
    if (choices.isEmpty()) {
        QString errMsg = tr("响应中没有 choices。");
        if (isTest) {
            emit connectionTestFinished(requestId, false, errMsg);
        } else {
            emit requestFailed(requestId, errMsg);
        }
        return;
    }

    QJsonObject firstChoice = choices[0].toObject();
    QJsonObject messageObj = firstChoice["message"].toObject();
    QString content = messageObj["content"].toString();

    if (isTest) {
        // Connection test succeeded if we got any content back
        emit connectionTestFinished(requestId, true, tr("连接成功。"));
    } else {
        if (content.isEmpty()) {
            emit requestFailed(requestId, tr("响应内容为空。"));
            return;
        }
        emit requestFinished(requestId, content);
    }
}

QString ChatCompletionService::normalizeNetworkError(QNetworkReply *reply, const QByteArray &body)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString bodyStr = SecretStorageService::redactSecrets(QString::fromUtf8(body));

    if (status > 0) {
        QString hint = statusHint(status);
        QString result = tr("HTTP %1：%2").arg(status).arg(hint);

        // Try to extract provider error message from body
        if (!bodyStr.isEmpty()) {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
            if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
                QString apiMsg = extractApiErrorMessage(doc.object());
                if (!apiMsg.isEmpty()) {
                    result += "\n" + tr("服务返回：%1").arg(apiMsg);
                }
            }
        }

        return result;
    }

    // No HTTP status - network level error
    return SecretStorageService::redactSecrets(reply->errorString());
}

QString ChatCompletionService::extractApiErrorMessage(const QJsonObject &root)
{
    // Try root["error"]["message"]
    if (root.contains("error")) {
        QJsonValue errorVal = root["error"];
        if (errorVal.isObject()) {
            QString msg = errorVal.toObject()["message"].toString();
            if (!msg.isEmpty()) {
                return SecretStorageService::redactSecrets(msg);
            }
        }
    }

    // Try root["message"]
    if (root.contains("message")) {
        QString msg = root["message"].toString();
        if (!msg.isEmpty()) {
            return SecretStorageService::redactSecrets(msg);
        }
    }

    // Try root["error_description"]
    if (root.contains("error_description")) {
        QString msg = root["error_description"].toString();
        if (!msg.isEmpty()) {
            return SecretStorageService::redactSecrets(msg);
        }
    }

    return QString();
}

QString ChatCompletionService::statusHint(int statusCode)
{
    switch (statusCode) {
    case 400: return tr("请求参数错误，请检查模型名和请求格式。");
    case 401: return tr("认证失败，请检查 API Key。");
    case 403: return tr("访问被拒绝，请检查 API Key 权限或账户状态。");
    case 404: return tr("接口或模型不存在，请检查 Base URL 和模型名。");
    case 408: return tr("请求超时。");
    case 429: return tr("请求过于频繁或额度不足，请稍后再试。");
    case 500:
    case 502:
    case 503:
    case 504: return tr("服务端暂时不可用，请稍后再试。");
    default:  return tr("未知错误。");
    }
}

QString ChatCompletionService::generateRequestId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(12);
}
