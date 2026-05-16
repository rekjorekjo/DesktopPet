#include "chatsettingsservice.h"

#include "core/petpaths.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

ChatSettingsService &ChatSettingsService::instance()
{
    static ChatSettingsService svc;
    return svc;
}

ChatSettingsService::ChatSettingsService()
    : m_systemPrompt(defaultSystemPrompt())
{
}

// ── load / save ────────────────────────────────────────────────────

bool ChatSettingsService::load(QString *error)
{
    const QString filePath = PetPaths::chatSettingsFilePath();

    if (!QFile::exists(filePath)) {
        m_systemPrompt = defaultSystemPrompt();
        return true;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QObject::tr("无法打开文件：%1").arg(filePath);
        qWarning() << "ChatSettingsService: failed to open" << filePath << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error)
            *error = QObject::tr("JSON 解析失败：%1").arg(parseError.errorString());
        qWarning() << "ChatSettingsService: JSON parse error at offset"
                    << parseError.offset << parseError.errorString();
        return false;
    }

    QJsonObject root = doc.object();

    QString prompt = root["systemPrompt"].toString().trimmed();
    if (prompt.isEmpty()) {
        m_systemPrompt = defaultSystemPrompt();
    } else {
        m_systemPrompt = prompt;
    }

    return true;
}

bool ChatSettingsService::save(QString *error) const
{
    const QString filePath = PetPaths::chatSettingsFilePath();
    const QString dirPath = PetPaths::configDir();

    QDir dir;
    if (!dir.exists(dirPath)) {
        if (!dir.mkpath(dirPath)) {
            if (error)
                *error = QObject::tr("无法创建目录：%1").arg(dirPath);
            qWarning() << "ChatSettingsService: failed to create dir" << dirPath;
            return false;
        }
    }

    QJsonObject root;
    root["version"] = 1;
    root["systemPrompt"] = m_systemPrompt;

    QJsonDocument doc(root);

    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QObject::tr("无法写入文件：%1").arg(filePath);
        qWarning() << "ChatSettingsService: failed to open save file" << filePath
                    << saveFile.errorString();
        return false;
    }

    if (saveFile.write(doc.toJson(QJsonDocument::Indented)) == -1) {
        if (error)
            *error = QObject::tr("写入文件失败：%1").arg(saveFile.errorString());
        qWarning() << "ChatSettingsService: write failed" << saveFile.errorString();
        return false;
    }

    if (!saveFile.commit()) {
        if (error)
            *error = QObject::tr("提交文件失败：%1").arg(saveFile.errorString());
        qWarning() << "ChatSettingsService: commit failed" << saveFile.errorString();
        return false;
    }

    return true;
}

// ── accessors ──────────────────────────────────────────────────────

QString ChatSettingsService::systemPrompt() const
{
    return m_systemPrompt;
}

void ChatSettingsService::setSystemPrompt(const QString &prompt)
{
    m_systemPrompt = prompt;
}

void ChatSettingsService::resetToDefaultSystemPrompt()
{
    m_systemPrompt = defaultSystemPrompt();
}

QString ChatSettingsService::defaultSystemPrompt()
{
    return QObject::tr(
        "你是 DesktopPet 的桌面宠物助手。请扮演一只陪伴用户的可爱桌宠。\n"
        "你的回复应简洁、自然、友好，带一点宠物感，需要有自己的情绪，如开心、惊讶、害怕、疑问、愤怒等。\n"
        "当用户需要帮助时，认真回答；当用户闲聊时，正常陪伴。\n"
        "默认使用中文回复。");
}
