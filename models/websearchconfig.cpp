#include "websearchconfig.h"

#include <QJsonObject>

QString webSearchProviderToString(WebSearchProvider provider)
{
    switch (provider) {
    case WebSearchProvider::Tavily: return "tavily";
    case WebSearchProvider::Brave:  return "brave";
    case WebSearchProvider::Exa:    return "exa";
    }
    return "tavily";
}

WebSearchProvider webSearchProviderFromString(const QString &text)
{
    if (text == "brave")  return WebSearchProvider::Brave;
    if (text == "exa")    return WebSearchProvider::Exa;
    return WebSearchProvider::Tavily;
}

QJsonObject webSearchConfigToJson(const WebSearchConfig &config)
{
    QJsonObject obj;
    obj["enabled"] = config.enabled;
    obj["provider"] = webSearchProviderToString(config.provider);
    obj["apiKey"] = config.apiKey;
    obj["resultCount"] = config.resultCount;
    obj["searchDepth"] = config.searchDepth;
    obj["timeoutMs"] = config.timeoutMs;
    return obj;
}

WebSearchConfig webSearchConfigFromJson(const QJsonObject &obj)
{
    WebSearchConfig config;
    config.enabled = obj["enabled"].toBool(false);
    config.provider = webSearchProviderFromString(obj["provider"].toString());
    config.apiKey = obj["apiKey"].toString();
    config.resultCount = qBound(1, obj["resultCount"].toInt(5), 10);
    config.searchDepth = obj["searchDepth"].toString("basic");
    if (config.searchDepth != "basic" && config.searchDepth != "advanced") {
        config.searchDepth = "basic";
    }
    config.timeoutMs = qBound(5000, obj["timeoutMs"].toInt(15000), 60000);
    return config;
}
