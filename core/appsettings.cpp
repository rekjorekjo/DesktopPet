#include "appsettings.h"

#include <QSettings>

double AppSettings::petScaleFactor()
{
    QSettings settings("DesktopPet", "DesktopPet");
    double scale = settings.value("display/petScaleFactor", 1.0).toDouble();
    return clampScale(scale);
}

void AppSettings::setPetScaleFactor(double scale)
{
    scale = clampScale(scale);
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("display/petScaleFactor", scale);
}

double AppSettings::clampScale(double scale)
{
    if (scale < 0.5) {
        return 0.5;
    }
    if (scale > 2.0) {
        return 2.0;
    }
    return scale;
}
