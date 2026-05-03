#ifndef APPSETTINGS_H
#define APPSETTINGS_H

class AppSettings
{
public:
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

private:
    static double clampScale(double scale);
    static double clampOpacity(double opacity);
};

#endif // APPSETTINGS_H
