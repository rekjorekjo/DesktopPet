#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QString>

class AppSettings
{
public:
    static constexpr int BaseMoveSpeedMin = 1;
    static constexpr int BaseMoveSpeedMax = 100;
    static constexpr int BaseMoveSpeedDefault = 20;

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

    static int cardGradientStrength();
    static void setCardGradientStrength(int value);

    static bool randomCardGradientEnabled();
    static void setRandomCardGradientEnabled(bool enabled);

    static int baseMoveSpeed();
    static void setBaseMoveSpeed(int speed);

private:
    static double clampScale(double scale);
    static double clampOpacity(double opacity);
    static int clampGradientStrength(int value);
    static int clampBaseMoveSpeed(int speed);
};

#endif // APPSETTINGS_H
