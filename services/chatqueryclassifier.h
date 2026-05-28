#ifndef CHATQUERYCLASSIFIER_H
#define CHATQUERYCLASSIFIER_H

#include <QString>
#include <QStringList>

class ChatQueryClassifier
{
public:
    static bool isForcedSearchQuery(const QString &text);
    static bool isLocalTimeQuery(const QString &text);
    static bool shouldUseWebSearch(const QString &userText, bool webSearchEnabled);
    static bool shouldUsePersonaRealtimeSearch(const QString &personaText, bool webSearchEnabled, bool optionEnabled);
    static QString buildPersonaRealtimeSearchQuery(const QString &personaText);
};

#endif // CHATQUERYCLASSIFIER_H
