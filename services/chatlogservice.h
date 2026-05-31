#ifndef CHATLOGSERVICE_H
#define CHATLOGSERVICE_H

#include <QDateTime>
#include <QList>
#include <QString>

struct ChatLogEntry
{
    QDateTime timestamp;
    QString petId;
    QString petName;
    QString apiConfigId;
    QString apiConfigName;
    QString provider;
    QString model;
    QString role;
    QString content;
    QString conversationId;
    bool error = false;
    QString errorMessage;
};

class ChatLogService
{
public:
    static QString rootLogDirectory();
    static QString petLogDirectory(const QString &petId);
    static QString logFilePath(const QString &petId, const QDate &date);

    static bool appendLog(const ChatLogEntry &entry);
    static QList<ChatLogEntry> loadLogFile(const QString &filePath);

    static QString formatEntriesForDisplay(const QList<ChatLogEntry> &entries);
    static QList<ChatLogEntry> filterEntries(const QList<ChatLogEntry> &entries, const QString &keyword);

private:
    static QString sanitizePetId(const QString &petId);
    static QJsonObject entryToJson(const ChatLogEntry &entry);
    static ChatLogEntry jsonToEntry(const QJsonObject &json);
    static QString formatRoleForEntry(const ChatLogEntry &entry);
};

#endif // CHATLOGSERVICE_H
