#include "test_websearchconfig.h"

#include <QtTest>
#include "models/websearchconfig.h"

void TestWebSearchConfig::providerToString_data()
{
    QTest::addColumn<WebSearchProvider>("provider");
    QTest::addColumn<QString>("expected");

    QTest::newRow("tavily") << WebSearchProvider::Tavily << "tavily";
    QTest::newRow("brave")  << WebSearchProvider::Brave  << "brave";
    QTest::newRow("exa")    << WebSearchProvider::Exa    << "exa";
}

void TestWebSearchConfig::providerToString()
{
    QFETCH(WebSearchProvider, provider);
    QFETCH(QString, expected);
    QCOMPARE(webSearchProviderToString(provider), expected);
}

void TestWebSearchConfig::providerFromString_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<WebSearchProvider>("expected");

    QTest::newRow("tavily")   << "tavily"   << WebSearchProvider::Tavily;
    QTest::newRow("brave")    << "brave"    << WebSearchProvider::Brave;
    QTest::newRow("exa")      << "exa"      << WebSearchProvider::Exa;
    QTest::newRow("unknown")  << "unknown"  << WebSearchProvider::Tavily;
    QTest::newRow("empty")    << ""          << WebSearchProvider::Tavily;
}

void TestWebSearchConfig::providerFromString()
{
    QFETCH(QString, input);
    QFETCH(WebSearchProvider, expected);
    QCOMPARE(webSearchProviderFromString(input), expected);
}

void TestWebSearchConfig::defaultConfig()
{
    WebSearchConfig config;
    QCOMPARE(config.enabled, false);
    QCOMPARE(config.provider, WebSearchProvider::Tavily);
    QCOMPARE(config.resultCount, 5);
    QCOMPARE(config.searchDepth, QString("basic"));
    QCOMPARE(config.timeoutMs, 15000);
}

void TestWebSearchConfig::jsonRoundTrip()
{
    WebSearchConfig original;
    original.enabled = true;
    original.provider = WebSearchProvider::Brave;
    original.apiKey = "test-key";
    original.resultCount = 7;
    original.searchDepth = "advanced";
    original.timeoutMs = 20000;

    QJsonObject json = webSearchConfigToJson(original);
    WebSearchConfig restored = webSearchConfigFromJson(json);

    QCOMPARE(restored.enabled, original.enabled);
    QCOMPARE(restored.provider, original.provider);
    QCOMPARE(restored.apiKey, original.apiKey);
    QCOMPARE(restored.resultCount, original.resultCount);
    QCOMPARE(restored.searchDepth, original.searchDepth);
    QCOMPARE(restored.timeoutMs, original.timeoutMs);
}

void TestWebSearchConfig::boundaryClamp_data()
{
    QTest::addColumn<int>("resultCount");
    QTest::addColumn<int>("timeoutMs");
    QTest::addColumn<QString>("searchDepth");
    QTest::addColumn<int>("expectedResultCount");
    QTest::addColumn<int>("expectedTimeoutMs");
    QTest::addColumn<QString>("expectedSearchDepth");

    QTest::newRow("too-high")
        << 999 << 100 << QString("bad-depth")
        << 10 << 5000 << QString("basic");

    QTest::newRow("too-low")
        << -1 << 999999 << QString("basic")
        << 1 << 60000 << QString("basic");
}

void TestWebSearchConfig::boundaryClamp()
{
    QFETCH(int, resultCount);
    QFETCH(int, timeoutMs);
    QFETCH(QString, searchDepth);
    QFETCH(int, expectedResultCount);
    QFETCH(int, expectedTimeoutMs);
    QFETCH(QString, expectedSearchDepth);

    QJsonObject obj;
    obj["resultCount"] = resultCount;
    obj["timeoutMs"] = timeoutMs;
    obj["searchDepth"] = searchDepth;

    WebSearchConfig config = webSearchConfigFromJson(obj);

    QCOMPARE(config.resultCount, expectedResultCount);
    QCOMPARE(config.timeoutMs, expectedTimeoutMs);
    QCOMPARE(config.searchDepth, expectedSearchDepth);
}

#include "moc_test_websearchconfig.cpp"
