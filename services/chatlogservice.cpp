#include "chatlogservice.h"

#include "core/petpaths.h"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringConverter>
#include <QTextStream>
#include <QDebug>

QString ChatLogService::rootLogDirectory()
{
    QString rootDir = PetPaths::rootDirectory() + "/logs/chat";
    QDir dir(rootDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return rootDir;
}

QString ChatLogService::petLogDirectory(const QString &petId)
{
    QString safePetId = sanitizePetId(petId);
    if (safePetId.isEmpty()) {
        safePetId = "default_pet";
    }

    QString petDir = rootLogDirectory() + "/" + safePetId;
    QDir dir(petDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return petDir;
}

QString ChatLogService::logFilePath(const QString &petId, const QDate &date)
{
    QString petDir = petLogDirectory(petId);
    QString fileName = date.toString("yyyyMMdd") + ".jsonl";
    return petDir + "/" + fileName;
}

bool ChatLogService::appendLog(const ChatLogEntry &entry)
{
    QString filePath = logFilePath(entry.petId, entry.timestamp.date());

    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "ChatLogService: Failed to open log file for writing:" << filePath;
        return false;
    }

    QJsonObject json = entryToJson(entry);
    QJsonDocument doc(json);
    QString jsonLine = QString::fromUtf8(doc.toJson(QJsonDocument::Compact)) + "\n";

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << jsonLine;
    out.flush();
    file.close();

    return true;
}

QList<ChatLogEntry> ChatLogService::loadLogFile(const QString &filePath)
{
    QList<ChatLogEntry> entries;

    QFile file(filePath);
    if (!file.exists()) {
        return entries;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ChatLogService: Failed to open log file for reading:" << filePath;
        return entries;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "ChatLogService: Failed to parse log line:" << parseError.errorString();
            continue;
        }

        if (!doc.isObject()) {
            qWarning() << "ChatLogService: Log line is not a JSON object";
            continue;
        }

        ChatLogEntry entry = jsonToEntry(doc.object());
        entries.append(entry);
    }

    file.close();
    return entries;
}

QString ChatLogService::formatEntriesForDisplay(const QList<ChatLogEntry> &entries)
{
    QString result;

    for (const ChatLogEntry &entry : entries) {
        QString timeStr = entry.timestamp.toString("HH:mm:ss");
        QString header = QString("[%1] 宠物：%2 | API：%3 | 模型：%4")
                             .arg(timeStr,
                                  entry.petName.isEmpty() ? entry.petId : entry.petName,
                                  entry.apiConfigName.isEmpty() ? entry.apiConfigId : entry.apiConfigName,
                                  entry.model);

        result += header + "\n";

        if (entry.error) {
            result += "错误：\n";
            result += entry.errorMessage.isEmpty() ? "未知错误" : entry.errorMessage;
        } else {
            QString roleDisplay = formatRole(entry.role);
            result += roleDisplay + "：\n";
            result += entry.content;
        }

        result += "\n\n";
    }

    return result.trimmed();
}

QList<ChatLogEntry> ChatLogService::filterEntries(const QList<ChatLogEntry> &entries, const QString &keyword)
{
    if (keyword.isEmpty()) {
        return entries;
    }

    QList<ChatLogEntry> filtered;
    QString lowerKeyword = keyword.toLower();

    for (const ChatLogEntry &entry : entries) {
        bool matches = false;

        if (entry.content.toLower().contains(lowerKeyword)) {
            matches = true;
        } else if (entry.petName.toLower().contains(lowerKeyword)) {
            matches = true;
        } else if (entry.apiConfigName.toLower().contains(lowerKeyword)) {
            matches = true;
        } else if (entry.provider.toLower().contains(lowerKeyword)) {
            matches = true;
        } else if (entry.model.toLower().contains(lowerKeyword)) {
            matches = true;
        } else if (entry.errorMessage.toLower().contains(lowerKeyword)) {
            matches = true;
        }

        if (matches) {
            filtered.append(entry);
        }
    }

    return filtered;
}

QString ChatLogService::sanitizePetId(const QString &petId)
{
    if (petId.isEmpty()) {
        return QString();
    }

    QString result = petId;

    result.replace("/", "_");
    result.replace("\\", "_");
    result.replace("..", "_");

    QString safeResult;
    for (const QChar &c : result) {
        if (c.isLetterOrNumber() || c == '_' || c == '-') {
            safeResult += c;
        } else if (c.isSpace()) {
            safeResult += "_";
        } else {
            safeResult += "_";
        }
    }

    return safeResult;
}

QJsonObject ChatLogService::entryToJson(const ChatLogEntry &entry)
{
    QJsonObject json;

    json["timestamp"] = entry.timestamp.toString(Qt::ISODate);
    json["petId"] = entry.petId;
    json["petName"] = entry.petName;
    json["apiConfigId"] = entry.apiConfigId;
    json["apiConfigName"] = entry.apiConfigName;
    json["provider"] = entry.provider;
    json["model"] = entry.model;
    json["role"] = entry.role;
    json["content"] = entry.content;
    json["conversationId"] = entry.conversationId;
    json["error"] = entry.error;
    json["errorMessage"] = entry.errorMessage;

    return json;
}

ChatLogEntry ChatLogService::jsonToEntry(const QJsonObject &json)
{
    ChatLogEntry entry;

    QString timestampStr = json["timestamp"].toString();
    entry.timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
    if (!entry.timestamp.isValid()) {
        entry.timestamp = QDateTime::currentDateTime();
    }

    entry.petId = json["petId"].toString();
    entry.petName = json["petName"].toString();
    entry.apiConfigId = json["apiConfigId"].toString();
    entry.apiConfigName = json["apiConfigName"].toString();
    entry.provider = json["provider"].toString();
    entry.model = json["model"].toString();
    entry.role = json["role"].toString();
    entry.content = json["content"].toString();
    entry.conversationId = json["conversationId"].toString();
    entry.error = json["error"].toBool(false);
    entry.errorMessage = json["errorMessage"].toString();

    return entry;
}

QString ChatLogService::formatRole(const QString &role)
{
    QString lowerRole = role.toLower();

    if (lowerRole == "user") {
        return "用户";
    } else if (lowerRole == "assistant") {
        return "AI";
    } else if (lowerRole == "system") {
        return "系统";
    }

    return role;
}
