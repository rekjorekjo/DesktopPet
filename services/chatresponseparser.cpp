#include "chatresponseparser.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

QStringList ChatResponseParser::defaultEmotions()
{
    return {"neutral", "happy", "sad", "angry", "surprised", "fear", "confused"};
}

QString ChatResponseParser::responseFormatInstruction(const QStringList &emotions)
{
    QString emotionList = emotions.join(", ");
    return QStringLiteral(
        "你必须只返回一个 JSON 对象，不要使用 Markdown 代码块，不要在 JSON 外输出其他文字。\n"
        "JSON 格式如下：\n"
        "{\"reply\":\"显示给用户的回复\",\"emotion\":\"neutral\"}\n"
        "emotion 只能从以下列表中选择：%1\n"
        "如果没有明显情绪，使用 neutral。"
    ).arg(emotionList);
}

ParsedChatResponse ChatResponseParser::parse(const QString &rawContent, const QStringList &allowedEmotions)
{
    ParsedChatResponse result;
    QString trimmed = rawContent.trimmed();

    // Try parsing as complete JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(trimmed.toUtf8(), &parseError);

    // If direct parse fails, try extracting first {...} block
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        static const QRegularExpression jsonRe(QStringLiteral("\\{[^{}]*\\}"));
        QRegularExpressionMatch match = jsonRe.match(trimmed);
        if (match.hasMatch()) {
            doc = QJsonDocument::fromJson(match.captured().toUtf8(), &parseError);
        }
    }

    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonObject obj = doc.object();
        QString reply = obj["reply"].toString().trimmed();
        QString emotion = obj["emotion"].toString().trimmed().toLower();

        if (reply.isEmpty()) {
            reply = trimmed;
        }
        if (emotion.isEmpty() || !allowedEmotions.contains(emotion)) {
            emotion = "neutral";
        }

        result.reply = reply;
        result.emotion = emotion;
        result.parsedAsJson = true;
    } else {
        result.reply = trimmed;
        result.emotion = "neutral";
        result.parsedAsJson = false;
    }

    return result;
}
