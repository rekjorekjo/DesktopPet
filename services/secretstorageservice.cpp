#include "secretstorageservice.h"

#include <QRegularExpression>
#include <QObject>

QString SecretStorageService::backendName()
{
    return QStringLiteral("Plain JSON");
}

bool SecretStorageService::usesSecureStorage()
{
    return false;
}

QString SecretStorageService::securityNotice()
{
    return QObject::tr(
        "当前版本 API Key 会保存在本地 config/api_profiles.json 中；"
        "不会上传给 DesktopPet 项目方；"
        "不要在公共电脑或不可信设备上保存密钥；"
        "后续版本计划支持系统级安全存储。");
}

QString SecretStorageService::maskSecret(const QString &secret)
{
    if (secret.isEmpty())
        return QString();

    if (secret.size() <= 8)
        return QStringLiteral("••••");

    return secret.left(4) + QStringLiteral("••••") + secret.right(4);
}

bool SecretStorageService::looksLikeSecret(const QString &text)
{
    if (text.isEmpty())
        return false;

    QString lower = text.toLower();
    return lower.contains("sk-")
        || lower.contains("bearer ")
        || lower.contains("api_key")
        || lower.contains("apikey")
        || lower.contains("authorization");
}

QString SecretStorageService::redactSecrets(const QString &text)
{
    if (text.isEmpty())
        return QString();

    QString result = text;

    // Redact Bearer tokens
    static QRegularExpression bearerRe(
        QStringLiteral("(Bearer\\s+)\\S+"),
        QRegularExpression::CaseInsensitiveOption);
    result.replace(bearerRe, QStringLiteral("\\1[REDACTED]"));

    // Redact sk-... tokens
    static QRegularExpression skRe(
        QStringLiteral("sk-\\S+"));
    result.replace(skRe, QStringLiteral("[REDACTED_API_KEY]"));

    return result;
}
