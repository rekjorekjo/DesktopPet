#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>

class AppSettings
{
public:
    static QString currentPetId();
    static void setCurrentPetId(const QString &petId);

    static double petScaleFactor();
    static void setPetScaleFactor(double scale);

    static double petOpacity();
    static void setPetOpacity(double opacity);

    static bool autoStartOnBoot();
    static void setAutoStartOnBoot(bool enabled);

    static bool autoPlayOnLaunch();
    static void setAutoPlayOnLaunch(bool enabled);

    static bool openSettingsOnLaunch();
    static void setOpenSettingsOnLaunch(bool enabled);

    static int themeIndex();
    static void setThemeIndex(int index);

private:
    static double clampScale(double scale);
    static double clampOpacity(double opacity);
};

#endif // APPSETTINGS_H
