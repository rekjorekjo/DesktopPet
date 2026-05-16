#ifndef APIPROVIDERPRESET_H
#define APIPROVIDERPRESET_H

#include "models/apiconfig.h"

#include <QList>
#include <QString>

struct ApiProviderPreset
{
    QString id;
    QString displayName;
    ApiFormat defaultFormat;
    QString defaultTemplate;
};

class ApiProviderPresetRegistry
{
public:
    static const QList<ApiProviderPreset> &presets();

    static const ApiProviderPreset *findById(const QString &id);
    static int indexOfId(const QString &id);
};

#endif // APIPROVIDERPRESET_H
