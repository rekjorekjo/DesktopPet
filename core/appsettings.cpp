#include "appsettings.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

QString AppSettings::currentPetId()
{
    QSettings settings("DesktopPet", "DesktopPet");
    return settings.value("pet/currentPetId", "default_pet").toString();
}

void AppSettings::setCurrentPetId(const QString &petId)
{
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("pet/currentPetId", petId);
}

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

double AppSettings::petOpacity()
{
    QSettings settings("DesktopPet", "DesktopPet");
    double opacity = settings.value("display/petOpacity", 1.0).toDouble();
    return clampOpacity(opacity);
}

void AppSettings::setPetOpacity(double opacity)
{
    opacity = clampOpacity(opacity);
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("display/petOpacity", opacity);
}

bool AppSettings::autoStartOnBoot()
{
    QSettings settings("DesktopPet", "DesktopPet");
    return settings.value("startup/autoStartOnBoot", false).toBool();
}

void AppSettings::setAutoStartOnBoot(bool enabled)
{
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("startup/autoStartOnBoot", enabled);

#ifdef Q_OS_WIN
    QSettings regSettings(
        "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat);
    if (enabled) {
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        regSettings.setValue("DesktopPet", QString("\"%1\"").arg(appPath));
    } else {
        regSettings.remove("DesktopPet");
    }
#else
    Q_UNUSED(enabled);
#endif
}

bool AppSettings::autoPlayOnLaunch()
{
    QSettings settings("DesktopPet", "DesktopPet");
    return settings.value("startup/autoPlayOnLaunch", true).toBool();
}

void AppSettings::setAutoPlayOnLaunch(bool enabled)
{
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("startup/autoPlayOnLaunch", enabled);
}

bool AppSettings::openSettingsOnLaunch()
{
    QSettings settings("DesktopPet", "DesktopPet");
    return settings.value("startup/openSettingsOnLaunch", false).toBool();
}

void AppSettings::setOpenSettingsOnLaunch(bool enabled)
{
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("startup/openSettingsOnLaunch", enabled);
}

int AppSettings::themeIndex()
{
    QSettings settings("DesktopPet", "DesktopPet");
    return settings.value("ui/themeIndex", 0).toInt();
}

void AppSettings::setThemeIndex(int index)
{
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("ui/themeIndex", index);
}

int AppSettings::cardGradientStrength()
{
    QSettings settings("DesktopPet", "DesktopPet");
    int value = settings.value("ui/cardGradientStrength", 35).toInt();
    return clampGradientStrength(value);
}

void AppSettings::setCardGradientStrength(int value)
{
    value = clampGradientStrength(value);
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("ui/cardGradientStrength", value);
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

double AppSettings::clampOpacity(double opacity)
{
    if (opacity < 0.2) {
        return 0.2;
    }
    if (opacity > 1.0) {
        return 1.0;
    }
    return opacity;
}

int AppSettings::clampGradientStrength(int value)
{
    if (value < 0) {
        return 0;
    }
    if (value > 100) {
        return 100;
    }
    return value;
}
