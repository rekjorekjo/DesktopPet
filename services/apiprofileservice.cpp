#include "apiprofileservice.h"

#include "core/appsettings.h"
#include "core/petpaths.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

ApiProfileService &ApiProfileService::instance()
{
    static ApiProfileService svc;
    return svc;
}

// ── load / save ────────────────────────────────────────────────────

bool ApiProfileService::load(QString *error)
{
    const QString filePath = PetPaths::apiProfilesFilePath();

    if (!QFile::exists(filePath)) {
        m_profiles.clear();
        m_currentProfileName.clear();
        return true;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QObject::tr("无法打开文件：%1").arg(filePath);
        qWarning() << "ApiProfileService: failed to open" << filePath << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (error)
            *error = QObject::tr("JSON 解析失败：%1").arg(parseError.errorString());
        qWarning() << "ApiProfileService: JSON parse error at offset"
                    << parseError.offset << parseError.errorString();
        return false;
    }

    QJsonObject root = doc.object();

    m_profiles.clear();
    m_currentProfileName.clear();

    QJsonArray profilesArray = root["profiles"].toArray();
    for (const QJsonValue &val : profilesArray) {
        if (!val.isObject())
            continue;

        QJsonObject obj = val.toObject();
        QString name = obj["name"].toString().trimmed();
        if (name.isEmpty())
            continue;

        ApiConfig cfg;
        if (configFromJson(obj, &cfg)) {
            m_profiles[name] = cfg;
        } else {
            qWarning() << "ApiProfileService: skipping invalid profile" << name;
        }
    }

    m_currentProfileName = root["currentProfile"].toString().trimmed();

    // Fallback: if JSON currentProfile is empty, try AppSettings
    if (m_currentProfileName.isEmpty()) {
        QString legacy = AppSettings::currentApiConfigName().trimmed();
        if (!legacy.isEmpty() && m_profiles.contains(legacy)) {
            m_currentProfileName = legacy;
        }
    }

    // Validate currentProfile exists in loaded profiles
    if (!m_currentProfileName.isEmpty() && !m_profiles.contains(m_currentProfileName)) {
        if (!m_profiles.isEmpty()) {
            m_currentProfileName = m_profiles.constBegin().key();
        } else {
            m_currentProfileName.clear();
        }
    }

    return true;
}

bool ApiProfileService::save(QString *error) const
{
    const QString filePath = PetPaths::apiProfilesFilePath();
    const QString dirPath = PetPaths::configDir();

    QDir dir;
    if (!dir.exists(dirPath)) {
        if (!dir.mkpath(dirPath)) {
            if (error)
                *error = QObject::tr("无法创建目录：%1").arg(dirPath);
            qWarning() << "ApiProfileService: failed to create dir" << dirPath;
            return false;
        }
    }

    QJsonObject root;
    root["version"] = 1;
    root["currentProfile"] = m_currentProfileName;

    QJsonArray profilesArray;
    for (auto it = m_profiles.constBegin(); it != m_profiles.constEnd(); ++it) {
        QJsonObject obj = configToJson(it.value());
        obj["name"] = it.key();
        profilesArray.append(obj);
    }
    root["profiles"] = profilesArray;

    QJsonDocument doc(root);

    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QObject::tr("无法写入文件：%1").arg(filePath);
        qWarning() << "ApiProfileService: failed to open save file" << filePath
                    << saveFile.errorString();
        return false;
    }

    if (saveFile.write(doc.toJson(QJsonDocument::Indented)) == -1) {
        if (error)
            *error = QObject::tr("写入文件失败：%1").arg(saveFile.errorString());
        qWarning() << "ApiProfileService: write failed" << saveFile.errorString();
        return false;
    }

    if (!saveFile.commit()) {
        if (error)
            *error = QObject::tr("提交文件失败：%1").arg(saveFile.errorString());
        qWarning() << "ApiProfileService: commit failed" << saveFile.errorString();
        return false;
    }

    return true;
}

// ── query ──────────────────────────────────────────────────────────

QList<ApiConfig> ApiProfileService::profiles() const
{
    return m_profiles.values();
}

QStringList ApiProfileService::profileNames() const
{
    return m_profiles.keys();
}

bool ApiProfileService::isEmpty() const
{
    return m_profiles.isEmpty();
}

bool ApiProfileService::hasProfile(const QString &name) const
{
    return m_profiles.contains(trimmedName(name));
}

bool ApiProfileService::profile(const QString &name, ApiConfig *outConfig) const
{
    auto it = m_profiles.find(trimmedName(name));
    if (it == m_profiles.end())
        return false;
    if (outConfig)
        *outConfig = it.value();
    return true;
}

// ── current profile ────────────────────────────────────────────────

QString ApiProfileService::currentProfileName() const
{
    return m_currentProfileName;
}

bool ApiProfileService::currentProfile(ApiConfig *outConfig) const
{
    if (m_currentProfileName.isEmpty())
        return false;
    return profile(m_currentProfileName, outConfig);
}

bool ApiProfileService::setCurrentProfileName(const QString &name, QString *error)
{
    QString trimmed = trimmedName(name);
    if (!trimmed.isEmpty() && !m_profiles.contains(trimmed)) {
        if (error)
            *error = QObject::tr("配置「%1」不存在。").arg(trimmed);
        return false;
    }

    m_currentProfileName = trimmed;
    AppSettings::setCurrentApiConfigName(m_currentProfileName);
    return save(error);
}

// ── CRUD ───────────────────────────────────────────────────────────

bool ApiProfileService::addProfile(const QString &name, const ApiConfig &config, QString *error)
{
    QString trimmed = trimmedName(name);
    if (trimmed.isEmpty()) {
        if (error)
            *error = QObject::tr("配置名称不能为空。");
        return false;
    }

    if (m_profiles.contains(trimmed)) {
        if (error)
            *error = QObject::tr("配置名称「%1」已存在。").arg(trimmed);
        return false;
    }

    m_profiles[trimmed] = config;
    return save(error);
}

bool ApiProfileService::updateProfile(const QString &oldName, const QString &newName,
                                      const ApiConfig &config, QString *error)
{
    QString trimmedOld = trimmedName(oldName);
    QString trimmedNew = trimmedName(newName);

    if (!m_profiles.contains(trimmedOld)) {
        if (error)
            *error = QObject::tr("配置「%1」不存在。").arg(trimmedOld);
        return false;
    }

    if (trimmedNew.isEmpty()) {
        if (error)
            *error = QObject::tr("配置名称不能为空。");
        return false;
    }

    // If name changed, check uniqueness
    if (trimmedNew != trimmedOld && m_profiles.contains(trimmedNew)) {
        if (error)
            *error = QObject::tr("配置名称「%1」已存在。").arg(trimmedNew);
        return false;
    }

    // Remove old entry if name changed
    if (trimmedNew != trimmedOld) {
        m_profiles.remove(trimmedOld);
    }

    m_profiles[trimmedNew] = config;

    // Sync current profile name if it was the old name
    if (m_currentProfileName == trimmedOld) {
        m_currentProfileName = trimmedNew;
        AppSettings::setCurrentApiConfigName(m_currentProfileName);
    }

    return save(error);
}

bool ApiProfileService::removeProfile(const QString &name, QString *error)
{
    QString trimmed = trimmedName(name);
    if (!m_profiles.contains(trimmed)) {
        if (error)
            *error = QObject::tr("配置「%1」不存在。").arg(trimmed);
        return false;
    }

    m_profiles.remove(trimmed);

    // If removed the current profile, pick the first remaining or clear
    if (m_currentProfileName == trimmed) {
        if (!m_profiles.isEmpty()) {
            m_currentProfileName = m_profiles.constBegin().key();
        } else {
            m_currentProfileName.clear();
        }
        AppSettings::setCurrentApiConfigName(m_currentProfileName);
    }

    return save(error);
}

void ApiProfileService::clear()
{
    m_profiles.clear();
    m_currentProfileName.clear();
}

// ── JSON serialization ─────────────────────────────────────────────

QJsonObject ApiProfileService::configToJson(const ApiConfig &config)
{
    QJsonObject obj;
    obj["providerId"] = config.providerId;
    obj["format"] = formatToString(config.apiFormat);
    obj["templateText"] = config.templateText;
    obj["apiKey"] = config.apiKey;
    obj["baseUrl"] = config.baseUrl;
    obj["model"] = config.model;
    obj["maxTokens"] = config.maxTokens;
    obj["temperature"] = config.temperature;
    return obj;
}

bool ApiProfileService::configFromJson(const QJsonObject &obj, ApiConfig *outConfig, QString *error)
{
    if (!outConfig) {
        if (error)
            *error = QObject::tr("内部错误：outConfig 为空。");
        return false;
    }

    outConfig->providerId = obj["providerId"].toString();
    outConfig->apiFormat = formatFromString(obj["format"].toString());
    outConfig->templateText = obj["templateText"].toString();
    outConfig->apiKey = obj["apiKey"].toString();
    outConfig->baseUrl = obj["baseUrl"].toString();
    outConfig->model = obj["model"].toString();
    outConfig->maxTokens = obj["maxTokens"].toInt(1024);
    outConfig->temperature = obj["temperature"].toDouble(0.7);
    return true;
}

QString ApiProfileService::formatToString(ApiFormat format)
{
    switch (format) {
    case ApiFormat::OpenAICompatible:
        return QStringLiteral("openai-compatible");
    case ApiFormat::AnthropicCompatible:
        return QStringLiteral("anthropic-compatible");
    case ApiFormat::Custom:
        return QStringLiteral("custom");
    }
    return QStringLiteral("custom");
}

ApiFormat ApiProfileService::formatFromString(const QString &value)
{
    if (value == "openai-compatible" || value == "openaicompatible")
        return ApiFormat::OpenAICompatible;
    if (value == "anthropic-compatible" || value == "anthropiccompatible")
        return ApiFormat::AnthropicCompatible;
    return ApiFormat::Custom;
}

QString ApiProfileService::trimmedName(const QString &name)
{
    return name.trimmed();
}
