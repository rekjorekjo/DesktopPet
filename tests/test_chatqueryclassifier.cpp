#include "test_chatqueryclassifier.h"

#include <QtTest>
#include "services/chatqueryclassifier.h"

void TestChatQueryClassifier::isLocalTimeQuery_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("now-what-time")     << QString("现在几点")       << true;
    QTest::newRow("now-what-time-ya")  << QString("现在几点呀")     << true;
    QTest::newRow("today-date")        << QString("今天几号")       << true;
    QTest::newRow("today-weekday")     << QString("今天星期几")     << true;
    QTest::newRow("current-time")      << QString("当前时间")       << true;
    QTest::newRow("is-evening")        << QString("现在是晚上吗")   << true;
    QTest::newRow("is-morning")        << QString("现在是上午")     << true;
    QTest::newRow("is-afternoon")      << QString("现在是下午")     << true;
    QTest::newRow("is-daytime")        << QString("现在是白天")     << true;
    QTest::newRow("how-recently")      << QString("最近怎么样")     << false;
    QTest::newRow("today-news")        << QString("今天有什么新闻") << false;
}

void TestChatQueryClassifier::isLocalTimeQuery()
{
    QFETCH(QString, input);
    QFETCH(bool, expected);
    QCOMPARE(ChatQueryClassifier::isLocalTimeQuery(input), expected);
}

void TestChatQueryClassifier::shouldUseWebSearchEnabled_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("now-what-time")        << QString("现在几点")             << false;
    QTest::newRow("today-date")           << QString("今天几号")             << false;
    QTest::newRow("today-news")           << QString("今天有什么新闻")       << true;
    QTest::newRow("how-recently")         << QString("最近怎么样")           << false;
    QTest::newRow("recently-news")        << QString("最近有什么新闻")       << true;
    QTest::newRow("latest-qt-version")    << QString("最新 Qt 版本")         << true;
    QTest::newRow("latest-version")       << QString("最新版本")             << true;
    QTest::newRow("price")                << QString("价格")                 << true;
    QTest::newRow("find-qt-release")      << QString("查找 Qt 6.11 发布信息") << true;
    QTest::newRow("search-qt-release")    << QString("搜索 Qt 6.11 发布信息") << true;
}

void TestChatQueryClassifier::shouldUseWebSearchEnabled()
{
    QFETCH(QString, input);
    QFETCH(bool, expected);
    QCOMPARE(ChatQueryClassifier::shouldUseWebSearch(input, true), expected);
}

void TestChatQueryClassifier::shouldUseWebSearchDisabled()
{
    QCOMPARE(ChatQueryClassifier::shouldUseWebSearch("今天有什么新闻", false), false);
    QCOMPARE(ChatQueryClassifier::shouldUseWebSearch("最新版本", false), false);
}

void TestChatQueryClassifier::shouldUsePersonaRealtimeSearch_data()
{
    QTest::addColumn<QString>("persona");
    QTest::addColumn<bool>("webEnabled");
    QTest::addColumn<bool>("optionEnabled");
    QTest::addColumn<bool>("expected");

    QTest::newRow("capybara-enabled")
        << QString("请扮演最近很火的卡通角色水豚噜噜")
        << true << true << true;

    QTest::newRow("plain-persona")
        << QString("请使用中文回答")
        << true << true << false;

    QTest::newRow("web-disabled")
        << QString("最近很火的卡通角色")
        << false << true << false;

    QTest::newRow("option-disabled")
        << QString("最近很火的卡通角色")
        << true << false << false;
}

void TestChatQueryClassifier::shouldUsePersonaRealtimeSearch()
{
    QFETCH(QString, persona);
    QFETCH(bool, webEnabled);
    QFETCH(bool, optionEnabled);
    QFETCH(bool, expected);
    QCOMPARE(ChatQueryClassifier::shouldUsePersonaRealtimeSearch(persona, webEnabled, optionEnabled), expected);
}

void TestChatQueryClassifier::buildPersonaRealtimeSearchQuery_data()
{
    QTest::addColumn<QString>("persona");
    QTest::addColumn<QString>("expectedContains1");
    QTest::addColumn<QString>("expectedContains2");

    QTest::newRow("capybara")
        << QString("你是 DesktopPet 的桌面宠物助手。请扮演最近很火的卡通角色水豚噜噜。默认中文回答。")
        << QString("最近很火")
        << QString("水豚噜噜");

    QTest::newRow("no-match")
        << QString("请使用中文回答所有问题。")
        << QString("")
        << QString("");
}

void TestChatQueryClassifier::buildPersonaRealtimeSearchQuery()
{
    QFETCH(QString, persona);
    QFETCH(QString, expectedContains1);
    QFETCH(QString, expectedContains2);

    QString result = ChatQueryClassifier::buildPersonaRealtimeSearchQuery(persona);

    if (expectedContains1.isEmpty()) {
        QVERIFY(result.isEmpty());
    } else {
        QVERIFY(result.contains(expectedContains1));
        QVERIFY(result.contains(expectedContains2));
    }
}

void TestChatQueryClassifier::isForcedSearchQuery_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("slash-search-space")     << QString("/search Qt 6.11")              << true;
    QTest::newRow("slash-search-no-space")  << QString("/searchQt 6.11")               << true;
    QTest::newRow("mid-message-search")     << QString("你好 /search Qt 6.11")         << true;
    QTest::newRow("emoji-search")           << QString("你好😳/searchQt 6.11")         << true;
    QTest::newRow("hash-search")            << QString("#search Qt 6.11")              << true;
    QTest::newRow("souyixia")               << QString("搜一下Qt 6.11")                << true;
    QTest::newRow("chayixia")               << QString("查一下Qt 6.11")                << true;
    QTest::newRow("plain-text")             << QString("今天有什么新闻")                << false;
    QTest::newRow("time-query")             << QString("现在几点")                     << false;
}

void TestChatQueryClassifier::isForcedSearchQuery()
{
    QFETCH(QString, input);
    QFETCH(bool, expected);
    QCOMPARE(ChatQueryClassifier::isForcedSearchQuery(input), expected);
}

void TestChatQueryClassifier::extractForcedSearchQuery_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("emoji-search-query")
        << QString("你好😳/search噜噜宝宝有没有喜欢吃的零食呢")
        << QString("噜噜宝宝有没有喜欢吃的零食呢");

    QTest::newRow("slash-search-space-query")
        << QString("/search Qt 6.11 发布信息")
        << QString("Qt 6.11 发布信息");

    QTest::newRow("trailing-command")
        << QString("你好 /search")
        << QString("你好");
}

void TestChatQueryClassifier::extractForcedSearchQuery()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QCOMPARE(ChatQueryClassifier::extractForcedSearchQuery(input), expected);
}

#include "moc_test_chatqueryclassifier.cpp"
