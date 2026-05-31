#ifndef TEST_CHATRESPONSEPARSER_H
#define TEST_CHATRESPONSEPARSER_H

#include <QObject>

class TestChatResponseParser : public QObject
{
    Q_OBJECT

private slots:
    void parseValidJson();
    void parseNeutral();
    void invalidEmotionFallsBackToNeutral();
    void missingReplyFallsBackToRaw();
    void markdownWrappedJson();
    void plainTextFallback();
};

#endif // TEST_CHATRESPONSEPARSER_H
