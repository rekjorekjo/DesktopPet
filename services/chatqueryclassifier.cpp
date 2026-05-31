#include "chatqueryclassifier.h"

#include <QRegularExpression>

bool ChatQueryClassifier::isForcedSearchQuery(const QString &text)
{
    return !extractForcedSearchQuery(text).isEmpty();
}

QString ChatQueryClassifier::extractForcedSearchQuery(const QString &text)
{
    QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }

    struct Trigger {
        QString marker;
        bool caseInsensitive;
    };
    static const QVector<Trigger> triggers = {
        {"/search", true},
        {"#search", true},
        {"搜一下", false},
        {"查一下", false},
    };

    int bestPos = -1;
    int bestMarkerLen = 0;

    for (const Trigger &t : triggers) {
        Qt::CaseSensitivity cs = t.caseInsensitive ? Qt::CaseInsensitive : Qt::CaseSensitive;
        int pos = trimmed.indexOf(t.marker, 0, cs);
        if (pos >= 0 && (bestPos < 0 || pos < bestPos)) {
            bestPos = pos;
            bestMarkerLen = t.marker.size();
        }
    }

    if (bestPos < 0) {
        return QString();
    }

    QString after = trimmed.mid(bestPos + bestMarkerLen).trimmed();
    if (!after.isEmpty()) {
        return after;
    }

    // Command with nothing after it: return everything before the command
    QString before = trimmed.left(bestPos).trimmed();
    return before.isEmpty() ? trimmed : before;
}

bool ChatQueryClassifier::isLocalTimeQuery(const QString &text)
{
    static const QStringList timePatterns = {
        QStringLiteral("现在几点"),
        QStringLiteral("几点了"),
        QStringLiteral("现在几时"),
        QStringLiteral("今天几号"),
        QStringLiteral("今天日期"),
        QStringLiteral("今天星期几"),
        QStringLiteral("星期几"),
        QStringLiteral("今天周几"),
        QStringLiteral("当前时间"),
        QStringLiteral("现在时间"),
        QStringLiteral("什么时间"),
        QStringLiteral("什么时候了"),
        QStringLiteral("现在是上午"),
        QStringLiteral("现在是下午"),
        QStringLiteral("现在是晚上"),
        QStringLiteral("现在是白天"),
    };

    QString trimmed = text.trimmed();
    for (const QString &pattern : timePatterns) {
        if (trimmed.contains(pattern)) {
            return true;
        }
    }
    return false;
}

bool ChatQueryClassifier::shouldUseWebSearch(const QString &userText, bool webSearchEnabled)
{
    if (!webSearchEnabled) {
        return false;
    }

    // Local time queries should never trigger search
    if (isLocalTimeQuery(userText)) {
        return false;
    }

    // Must NOT let these standalone words trigger search:
    //   最近, 现在, 当前, 今天
    // So we only include compound phrases or specific keywords.
    static const QStringList keywords = {
        QStringLiteral("今天有什么新闻"),
        QStringLiteral("最近有什么新闻"),
        QStringLiteral("最新新闻"),
        QStringLiteral("最新消息"),
        QStringLiteral("最新版本"),
        QStringLiteral("最近发布"),
        QStringLiteral("最近发布的"),
        QStringLiteral("天气"),
        QStringLiteral("价格"),
        QStringLiteral("股价"),
        QStringLiteral("汇率"),
        QStringLiteral("新闻"),
        QStringLiteral("版本"),
        QStringLiteral("发布"),
        QStringLiteral("实时"),
        QStringLiteral("联网"),
        QStringLiteral("搜索"),
        QStringLiteral("查找"),
        QStringLiteral("热搜"),
        QStringLiteral("头条"),
    };

    for (const QString &kw : keywords) {
        if (userText.contains(kw)) {
            return true;
        }
    }

    return false;
}

bool ChatQueryClassifier::shouldUsePersonaRealtimeSearch(const QString &personaText, bool webSearchEnabled, bool optionEnabled)
{
    if (!webSearchEnabled || !optionEnabled) {
        return false;
    }

    static const QStringList realtimeKeywords = {
        QStringLiteral("最近"),
        QStringLiteral("最新"),
        QStringLiteral("当前"),
        QStringLiteral("流行"),
        QStringLiteral("很火"),
        QStringLiteral("新闻"),
        QStringLiteral("天气"),
        QStringLiteral("版本"),
        QStringLiteral("价格"),
        QStringLiteral("发布"),
        QStringLiteral("实时"),
        QStringLiteral("热搜"),
    };

    for (const QString &kw : realtimeKeywords) {
        if (personaText.contains(kw)) {
            return true;
        }
    }

    return false;
}

QString ChatQueryClassifier::buildPersonaRealtimeSearchQuery(const QString &personaText)
{
    static const QStringList realtimeKeywords = {
        QStringLiteral("最近"),
        QStringLiteral("最新"),
        QStringLiteral("当前"),
        QStringLiteral("流行"),
        QStringLiteral("很火"),
        QStringLiteral("新闻"),
        QStringLiteral("天气"),
        QStringLiteral("版本"),
        QStringLiteral("价格"),
        QStringLiteral("发布"),
        QStringLiteral("实时"),
        QStringLiteral("热搜"),
    };

    // Split persona into sentences by Chinese/English punctuation
    static const QRegularExpression splitter(
        QStringLiteral("[。.；;\\n]+"));
    QStringList sentences = personaText.split(splitter, Qt::SkipEmptyParts);

    for (const QString &sentence : sentences) {
        QString trimmed = sentence.trimmed();
        if (trimmed.isEmpty()) continue;

        for (const QString &kw : realtimeKeywords) {
            if (trimmed.contains(kw)) {
                if (trimmed.size() > 80) {
                    return trimmed.left(80);
                }
                return trimmed;
            }
        }
    }

    return QString();
}
