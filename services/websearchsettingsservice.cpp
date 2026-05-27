#include "websearchsettingsservice.h"

#include "core/petpaths.h"
#include "services/secretstorageservice.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QSaveFile>

WebSearchConfig WebSearchSettingsService::defaultConfig()
{
    return WebSearchConfig();
}

WebSearchConfig WebSearchSettingsService::load(QString *errorMessage)
{
    const QString filePath = PetPaths::webSearchSettingsFilePath();

    if (!QFile::exists(filePath)) {
        return defaultConfig();
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        if (errorMessage)
            *errorMessage = QObject::tr("无法打开文件：%1").arg(filePath);
        qWarning() << "WebSearchSettingsService: failed to open" << filePath << file.errorString();
        return defaultConfig();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        if (errorMessage)
            *errorMessage = QObject::tr("JSON 解析失败：%1").arg(parseError.errorString());
        qWarning() << "WebSearchSettingsService: JSON parse error at offset"
                    << parseError.offset << parseError.errorString();
        return defaultConfig();
    }

    return webSearchConfigFromJson(doc.object());
}

bool WebSearchSettingsService::save(const WebSearchConfig &config, QString *errorMessage)
{
    const QString filePath = PetPaths::webSearchSettingsFilePath();
    const QString dirPath = PetPaths::configDir();

    QDir dir;
    if (!dir.exists(dirPath)) {
        if (!dir.mkpath(dirPath)) {
            if (errorMessage)
                *errorMessage = QObject::tr("无法创建目录：%1").arg(dirPath);
            qWarning() << "WebSearchSettingsService: failed to create dir" << dirPath;
            return false;
        }
    }

    QJsonObject root = webSearchConfigToJson(config);
    QJsonDocument doc(root);

    QSaveFile saveFile(filePath);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        if (errorMessage)
            *errorMessage = QObject::tr("无法写入文件：%1").arg(filePath);
        qWarning() << "WebSearchSettingsService: failed to open save file" << filePath
                    << saveFile.errorString();
        return false;
    }

    if (saveFile.write(doc.toJson(QJsonDocument::Indented)) == -1) {
        if (errorMessage)
            *errorMessage = QObject::tr("写入文件失败：%1").arg(saveFile.errorString());
        qWarning() << "WebSearchSettingsService: write failed" << saveFile.errorString();
        return false;
    }

    if (!saveFile.commit()) {
        if (errorMessage)
            *errorMessage = QObject::tr("提交文件失败：%1").arg(saveFile.errorString());
        qWarning() << "WebSearchSettingsService: commit failed" << saveFile.errorString();
        return false;
    }

    return true;
}
