#include "apiconfig.h"

#include <QStringList>

ApiConfig parseTemplateToConfig(const QString &templateText, ApiFormat format)
{
    ApiConfig config;
    config.apiFormat = format;
    config.templateText = templateText;

    const QStringList lines = templateText.split('\n', Qt::SkipEmptyParts);

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        int eqIdx = line.indexOf('=');
        if (eqIdx < 0)
            continue;

        QString key = line.left(eqIdx).trimmed().toUpper();
        QString value = line.mid(eqIdx + 1).trimmed();

        if (key == "API_KEY" || key == "AUTH_TOKEN") {
            config.apiKey = value;
        } else if (key == "BASE_URL") {
            config.baseUrl = value;
        } else if (key == "MODEL") {
            config.model = value;
        } else if (key == "MAX_TOKENS") {
            bool ok = false;
            int v = value.toInt(&ok);
            if (ok)
                config.maxTokens = clampMaxTokens(v);
        } else if (key == "TEMPERATURE") {
            bool ok = false;
            double v = value.toDouble(&ok);
            if (ok && v >= 0.0 && v <= 2.0)
                config.temperature = v;
        }
    }

    return config;
}

QString generateTemplateFromConfig(const ApiConfig &config)
{
    QString text;
    if (!config.apiKey.isEmpty())
        text += "API_KEY=" + config.apiKey + "\n";
    else
        text += "API_KEY=\n";

    if (!config.baseUrl.isEmpty())
        text += "BASE_URL=" + config.baseUrl + "\n";
    else
        text += "BASE_URL=\n";

    if (!config.model.isEmpty())
        text += "MODEL=" + config.model + "\n";
    else
        text += "MODEL=\n";

    return text;
}
