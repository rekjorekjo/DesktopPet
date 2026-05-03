#ifndef APPSETTINGS_H
#define APPSETTINGS_H

class AppSettings
{
public:
    static double petScaleFactor();
    static void setPetScaleFactor(double scale);

private:
    static double clampScale(double scale);
};

#endif // APPSETTINGS_H
