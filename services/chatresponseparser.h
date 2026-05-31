#ifndef CHATRESPONSEPARSER_H
#define CHATRESPONSEPARSER_H

#include <QString>
#include <QStringList>

struct ParsedChatResponse
{
    QString reply;
    QString emotion;
    bool parsedAsJson = false;
};

class ChatResponseParser
{
public:
    static QString responseFormatInstruction(const QStringList &emotions);
    static ParsedChatResponse parse(const QString &rawContent, const QStringList &allowedEmotions);

    static QStringList defaultEmotions();
};

#endif // CHATRESPONSEPARSER_H
