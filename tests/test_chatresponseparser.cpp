#include "test_chatresponseparser.h"

#include <QtTest>

#include "services/chatresponseparser.h"

void TestChatResponseParser::parseValidJson()
{
    QString input = R"({"reply":"噜噜喜欢红萝卜干！","emotion":"happy"})";
    ParsedChatResponse result = ChatResponseParser::parse(input, ChatResponseParser::defaultEmotions());

    QCOMPARE(result.reply, QString("噜噜喜欢红萝卜干！"));
    QCOMPARE(result.emotion, QString("happy"));
    QVERIFY(result.parsedAsJson);
}

void TestChatResponseParser::parseNeutral()
{
    QString input = R"({"reply":"我会陪着你。","emotion":"neutral"})";
    ParsedChatResponse result = ChatResponseParser::parse(input, ChatResponseParser::defaultEmotions());

    QCOMPARE(result.emotion, QString("neutral"));
    QVERIFY(result.parsedAsJson);
}

void TestChatResponseParser::invalidEmotionFallsBackToNeutral()
{
    QString input = R"({"reply":"你好","emotion":"excited"})";
    ParsedChatResponse result = ChatResponseParser::parse(input, ChatResponseParser::defaultEmotions());

    QCOMPARE(result.emotion, QString("neutral"));
    QVERIFY(result.parsedAsJson);
}

void TestChatResponseParser::missingReplyFallsBackToRaw()
{
    QString input = R"({"emotion":"happy"})";
    ParsedChatResponse result = ChatResponseParser::parse(input, ChatResponseParser::defaultEmotions());

    QVERIFY(!result.reply.isEmpty());
    QCOMPARE(result.emotion, QString("happy"));
    QVERIFY(result.parsedAsJson);
}

void TestChatResponseParser::markdownWrappedJson()
{
    QString input = "```json\n{\"reply\":\"你好呀\",\"emotion\":\"happy\"}\n```";
    ParsedChatResponse result = ChatResponseParser::parse(input, ChatResponseParser::defaultEmotions());

    QCOMPARE(result.reply, QString("你好呀"));
    QCOMPARE(result.emotion, QString("happy"));
    QVERIFY(result.parsedAsJson);
}

void TestChatResponseParser::plainTextFallback()
{
    QString input = "你好呀，我会陪着你。";
    ParsedChatResponse result = ChatResponseParser::parse(input, ChatResponseParser::defaultEmotions());

    QCOMPARE(result.reply, QString("你好呀，我会陪着你。"));
    QCOMPARE(result.emotion, QString("neutral"));
    QVERIFY(!result.parsedAsJson);
}

#include "moc_test_chatresponseparser.cpp"
