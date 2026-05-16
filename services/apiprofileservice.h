#ifndef APIPROFILESERVICE_H
#define APIPROFILESERVICE_H

#include "models/apiconfig.h"

#include <QJsonObject>
#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

class ApiProfileService
{
public:
    static ApiProfileService &instance();

    bool load(QString *error = nullptr);
    bool save(QString *error = nullptr) const;

    QList<ApiConfig> profiles() const;
    QStringList profileNames() const;
    bool isEmpty() const;
    bool hasProfile(const QString &name) const;

    bool profile(const QString &name, ApiConfig *outConfig) const;

    QString currentProfileName() const;
    bool currentProfile(ApiConfig *outConfig) const;
    bool setCurrentProfileName(const QString &name, QString *error = nullptr);

    bool addProfile(const QString &name, const ApiConfig &config, QString *error = nullptr);
    bool updateProfile(const QString &oldName, const QString &newName, const ApiConfig &config, QString *error = nullptr);
    bool removeProfile(const QString &name, QString *error = nullptr);

    void clear();

private:
    ApiProfileService() = default;

    static QJsonObject configToJson(const ApiConfig &config);
    static bool configFromJson(const QJsonObject &obj, ApiConfig *outConfig, QString *error = nullptr);

    static QString formatToString(ApiFormat format);
    static ApiFormat formatFromString(const QString &value);

    static QString trimmedName(const QString &name);

private:
    QMap<QString, ApiConfig> m_profiles;
    QString m_currentProfileName;
};

#endif // APIPROFILESERVICE_H
