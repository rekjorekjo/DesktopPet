#ifndef WEBSEARCHCONFIG_H
#define WEBSEARCHCONFIG_H

#include <QJsonObject>
#include <QString>

enum class WebSearchProvider
{
    Tavily,
    Brave,
    Exa
};

struct WebSearchConfig
{
    bool enabled = false;
    WebSearchProvider provider = WebSearchProvider::Tavily;
    QString apiKey;
    int resultCount = 5;
    QString searchDepth = "basic";
    int timeoutMs = 15000;
};

QString webSearchProviderToString(WebSearchProvider provider);
WebSearchProvider webSearchProviderFromString(const QString &text);

QJsonObject webSearchConfigToJson(const WebSearchConfig &config);
WebSearchConfig webSearchConfigFromJson(const QJsonObject &obj);

#endif // WEBSEARCHCONFIG_H
