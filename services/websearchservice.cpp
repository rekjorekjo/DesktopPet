#include "websearchservice.h"

#include "services/secretstorageservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUuid>
#include <QUrlQuery>

WebSearchService::WebSearchService(QObject *parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    connect(m_network, &QNetworkAccessManager::finished,
            this, &WebSearchService::onReplyFinished);
}

WebSearchService::~WebSearchService() {}

QString WebSearchService::search(const QString &query, const WebSearchConfig &config)
{
    const QString requestId = generateRequestId();

    if (!config.enabled) {
        QTimer::singleShot(0, this, [this, requestId, query]() {
            emit searchFailed(requestId, query, tr("联网搜索未启用。"));
        });
        return requestId;
    }

    if (config.apiKey.trimmed().isEmpty()) {
        QTimer::singleShot(0, this, [this, requestId, query]() {
            emit searchFailed(requestId, query, tr("搜索 API Key 未配置。"));
        });
        return requestId;
    }

    if (query.trimmed().isEmpty()) {
        QTimer::singleShot(0, this, [this, requestId, query]() {
            emit searchFailed(requestId, query, tr("搜索关键词为空。"));
        });
        return requestId;
    }

    QNetworkRequest request;
    QByteArray body;

    switch (config.provider) {
    case WebSearchProvider::Tavily: {
        QUrl url("https://api.tavily.com/search");
        request = QNetworkRequest(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", ("Bearer " + config.apiKey.trimmed()).toUtf8());

        QJsonObject obj;
        obj["query"] = query;
        obj["max_results"] = config.resultCount;
        obj["search_depth"] = config.searchDepth;
        obj["include_answer"] = false;
        obj["include_raw_content"] = false;
        obj["include_images"] = false;
        body = QJsonDocument(obj).toJson(QJsonDocument::Compact);
        break;
    }
    case WebSearchProvider::Brave: {
        QUrl url("https://api.search.brave.com/res/v1/web/search");
        QUrlQuery urlQuery;
        urlQuery.addQueryItem("q", query);
        urlQuery.addQueryItem("count", QString::number(config.resultCount));
        url.setQuery(urlQuery);

        request = QNetworkRequest(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Accept", "application/json");
        request.setRawHeader("Accept-Encoding", "gzip");
        request.setRawHeader("X-Subscription-Token", config.apiKey.trimmed().toUtf8());
        break;
    }
    case WebSearchProvider::Exa: {
        QUrl url("https://api.exa.ai/search");
        request = QNetworkRequest(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("x-api-key", config.apiKey.trimmed().toUtf8());

        QJsonObject contents;
        contents["highlights"] = true;

        QJsonObject obj;
        obj["query"] = query;
        obj["numResults"] = config.resultCount;
        obj["contents"] = contents;
        body = QJsonDocument(obj).toJson(QJsonDocument::Compact);
        break;
    }
    }

    QNetworkReply *reply;
    if (config.provider == WebSearchProvider::Brave) {
        reply = m_network->get(request);
    } else {
        reply = m_network->post(request, body);
    }

    auto *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, reply]() {
        onRequestTimeout(reply);
    });

    PendingSearch ps;
    ps.requestId = requestId;
    ps.query = query;
    ps.provider = config.provider;
    ps.timeoutTimer = timer;
    ps.canceled = false;
    m_pendingReplies.insert(reply, ps);
    m_repliesById.insert(requestId, reply);

    timer->start(config.timeoutMs);

    return requestId;
}

void WebSearchService::cancelRequest(const QString &requestId)
{
    QNetworkReply *reply = m_repliesById.value(requestId);
    if (!reply) return;

    auto it = m_pendingReplies.find(reply);
    if (it == m_pendingReplies.end()) return;

    QTimer *timer = it->timeoutTimer;

    m_pendingReplies.erase(it);
    m_repliesById.remove(requestId);

    if (timer) {
        timer->stop();
        timer->deleteLater();
    }

    reply->abort();
    reply->deleteLater();

    emit searchCanceled(requestId);
}

void WebSearchService::cancelAllRequests()
{
    const QStringList ids = m_repliesById.keys();
    for (const QString &id : ids) {
        cancelRequest(id);
    }
}

void WebSearchService::onRequestTimeout(QNetworkReply *reply)
{
    auto it = m_pendingReplies.find(reply);
    if (it == m_pendingReplies.end()) return;

    QString requestId = it->requestId;
    QString query = it->query;
    QTimer *timer = it->timeoutTimer;

    m_pendingReplies.erase(it);
    m_repliesById.remove(requestId);

    if (timer) {
        timer->stop();
        timer->deleteLater();
    }

    reply->abort();
    reply->deleteLater();

    emit searchFailed(requestId, query, tr("联网搜索请求超时。"));
}

void WebSearchService::onReplyFinished(QNetworkReply *reply)
{
    auto it = m_pendingReplies.find(reply);
    if (it == m_pendingReplies.end()) {
        reply->deleteLater();
        return;
    }

    PendingSearch ps = *it;
    m_pendingReplies.erase(it);
    m_repliesById.remove(ps.requestId);

    if (ps.timeoutTimer) {
        ps.timeoutTimer->stop();
        ps.timeoutTimer->deleteLater();
    }

    reply->deleteLater();

    if (ps.canceled) return;

    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            emit searchFailed(ps.requestId, ps.query, tr("联网搜索请求超时。"));
            return;
        }

        QByteArray body = reply->readAll();
        QString errorMsg = normalizeNetworkError(reply, body);
        emit searchFailed(ps.requestId, ps.query, errorMsg);
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit searchFailed(ps.requestId, ps.query,
                          tr("JSON 解析失败：%1").arg(parseError.errorString()));
        return;
    }

    if (!doc.isObject()) {
        emit searchFailed(ps.requestId, ps.query, tr("响应格式错误：非 JSON 对象。"));
        return;
    }

    QJsonObject root = doc.object();

    QList<WebSearchResult> results;
    switch (ps.provider) {
    case WebSearchProvider::Tavily:
        results = parseTavilyResponse(root);
        break;
    case WebSearchProvider::Brave:
        results = parseBraveResponse(root);
        break;
    case WebSearchProvider::Exa:
        results = parseExaResponse(root);
        break;
    }

    if (results.isEmpty()) {
        emit searchFailed(ps.requestId, ps.query, tr("没有找到可用的联网搜索结果。"));
        return;
    }

    emit searchFinished(ps.requestId, ps.query, results);
}

QString WebSearchService::normalizeNetworkError(QNetworkReply *reply, const QByteArray &body)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString bodyStr = SecretStorageService::redactSecrets(QString::fromUtf8(body));

    if (status > 0) {
        QString hint = statusHint(status);
        QString result = hint;

        if (!bodyStr.isEmpty()) {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);
            if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
                QString apiMsg = extractProviderErrorMessage(doc.object(), WebSearchProvider::Tavily);
                if (!apiMsg.isEmpty()) {
                    result += "\n" + tr("服务返回：%1").arg(apiMsg);
                }
            }
        }

        return result;
    }

    return SecretStorageService::redactSecrets(reply->errorString());
}

QString WebSearchService::extractProviderErrorMessage(const QJsonObject &root, WebSearchProvider provider)
{
    Q_UNUSED(provider);

    if (root.contains("error")) {
        QJsonValue errorVal = root["error"];
        if (errorVal.isObject()) {
            QString msg = errorVal.toObject()["message"].toString();
            if (!msg.isEmpty()) {
                return SecretStorageService::redactSecrets(msg);
            }
        } else if (errorVal.isString()) {
            return SecretStorageService::redactSecrets(errorVal.toString());
        }
    }

    if (root.contains("message")) {
        QString msg = root["message"].toString();
        if (!msg.isEmpty()) {
            return SecretStorageService::redactSecrets(msg);
        }
    }

    return QString();
}

QString WebSearchService::statusHint(int statusCode)
{
    switch (statusCode) {
    case 401: return tr("联网搜索认证失败，请检查搜索 API Key。");
    case 403: return tr("联网搜索访问被拒绝，请检查 API Key 权限或账户状态。");
    case 404: return tr("联网搜索接口不存在，请检查搜索服务配置。");
    case 408: return tr("联网搜索请求超时。");
    case 429: return tr("联网搜索请求过于频繁或额度不足，请稍后再试。");
    case 500:
    case 502:
    case 503:
    case 504: return tr("搜索服务暂时不可用，请稍后再试。");
    default:  return tr("联网搜索失败：HTTP %1").arg(statusCode);
    }
}

QList<WebSearchResult> WebSearchService::parseTavilyResponse(const QJsonObject &root)
{
    QList<WebSearchResult> results;
    QJsonArray arr = root["results"].toArray();
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        WebSearchResult r;
        r.title = obj["title"].toString();
        r.url = obj["url"].toString();
        r.snippet = obj["content"].toString();
        r.publishedAt = obj["published_date"].toString();
        results.append(r);
    }
    return results;
}

QList<WebSearchResult> WebSearchService::parseBraveResponse(const QJsonObject &root)
{
    QList<WebSearchResult> results;
    QJsonObject web = root["web"].toObject();
    QJsonArray arr = web["results"].toArray();
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        WebSearchResult r;
        r.title = obj["title"].toString();
        r.url = obj["url"].toString();
        r.snippet = obj["description"].toString();
        QJsonObject profile = obj["profile"].toObject();
        r.source = profile["name"].toString();
        r.publishedAt = obj["age"].toString();
        if (r.publishedAt.isEmpty()) {
            r.publishedAt = obj["page_age"].toString();
        }
        results.append(r);
    }
    return results;
}

QList<WebSearchResult> WebSearchService::parseExaResponse(const QJsonObject &root)
{
    QList<WebSearchResult> results;
    QJsonArray arr = root["results"].toArray();
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        WebSearchResult r;
        r.title = obj["title"].toString();
        r.url = obj["url"].toString();

        QJsonArray highlights = obj["highlights"].toArray();
        if (!highlights.isEmpty()) {
            r.snippet = highlights[0].toString();
        } else {
            r.snippet = obj["text"].toString();
        }

        r.publishedAt = obj["publishedDate"].toString();
        results.append(r);
    }
    return results;
}

QString WebSearchService::generateRequestId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).left(12);
}
