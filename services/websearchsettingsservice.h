#ifndef WEBSEARCHSETTINGSSERVICE_H
#define WEBSEARCHSETTINGSSERVICE_H

#include "models/websearchconfig.h"

#include <QString>

class WebSearchSettingsService
{
public:
    static WebSearchConfig defaultConfig();
    static WebSearchConfig load(QString *errorMessage = nullptr);
    static bool save(const WebSearchConfig &config, QString *errorMessage = nullptr);
};

#endif // WEBSEARCHSETTINGSSERVICE_H
