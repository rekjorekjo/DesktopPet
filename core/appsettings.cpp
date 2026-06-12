#include "appsettings.h"

#include <QCoreApplication>
#include <QDir>
#include <QSettings>

#ifdef Q_OS_WIN
#include <QSettings>
#endif

#ifdef Q_OS_WIN
static const char *kRunKeyPath = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const char *kStartupApprovedKeyPath = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run";
static const char *kValueName = "DesktopPet";
#endif

QString AppSettings::currentPetId()
{
    QSettings settings("DesktopPet", "DesktopPet");
    return settings.value("pet/currentPetId", "").toString();
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
#ifdef Q_OS_WIN
    // Read the real system state from the registry
    QSettings runSettings(kRunKeyPath, QSettings::NativeFormat);
    if (!runSettings.contains(kValueName)) {
        return false;
    }

    // Check StartupApproved to see if disabled by task manager / security software
    QSettings startupApprovedSettings(kStartupApprovedKeyPath, QSettings::NativeFormat);
    if (startupApprovedSettings.contains(kValueName)) {
        QByteArray approvedData = startupApprovedSettings.value(kValueName).toByteArray();
        // First byte 0x03 means disabled
        if (!approvedData.isEmpty() && static_cast<unsigned char>(approvedData.at(0)) == 0x03) {
            return false;
        }
    }

    return true;
#else
    QSettings settings("DesktopPet", "DesktopPet");
    return settings.value("startup/autoStartOnBoot", false).toBool();
#endif
}

bool AppSettings::setAutoStartOnBoot(bool enabled)
{
#ifdef Q_OS_WIN
    QSettings runSettings(kRunKeyPath, QSettings::NativeFormat);
    QSettings startupApprovedSettings(kStartupApprovedKeyPath, QSettings::NativeFormat);

    if (enabled) {
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        runSettings.setValue(kValueName, QString("\"%1\"").arg(appPath));
        // Clear any disabled state from StartupApproved
        startupApprovedSettings.remove(kValueName);
    } else {
        runSettings.remove(kValueName);
        startupApprovedSettings.remove(kValueName);
    }

    runSettings.sync();
    startupApprovedSettings.sync();

    if (runSettings.status() != QSettings::NoError) {
        return false;
    }

    // Only persist to our own QSettings after registry write succeeds
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("startup/autoStartOnBoot", enabled);
    return true;
#else
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("startup/autoStartOnBoot", enabled);
    return true;
#endif
}

bool AppSettings::autoPlayOnLaunch()
{
    QSettings settings("DesktopPet", "DesktopPet");
    return settings.value("startup/autoPlayOnLaunch", false).toBool();
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

bool AppSettings::randomCardGradientEnabled()
{
    QSettings settings("DesktopPet", "DesktopPet");
    return settings.value("ui/randomCardGradientEnabled", true).toBool();
}

void AppSettings::setRandomCardGradientEnabled(bool enabled)
{
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("ui/randomCardGradientEnabled", enabled);
}

int AppSettings::baseMoveSpeed()
{
    QSettings settings("DesktopPet", "DesktopPet");
    int speed = settings.value("ui/baseMoveSpeed", BaseMoveSpeedDefault).toInt();
    return clampBaseMoveSpeed(speed);
}

void AppSettings::setBaseMoveSpeed(int speed)
{
    speed = clampBaseMoveSpeed(speed);
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("ui/baseMoveSpeed", speed);
}

int AppSettings::clampBaseMoveSpeed(int speed)
{
    if (speed < BaseMoveSpeedMin) {
        return BaseMoveSpeedMin;
    }
    if (speed > BaseMoveSpeedMax) {
        return BaseMoveSpeedMax;
    }
    return speed;
}
