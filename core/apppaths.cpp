#include "apppaths.h"

#include <QCoreApplication>
#include <QDir>

namespace AppPaths {

QString appDir()
{
    return QCoreApplication::applicationDirPath();
}

QString dataRootDir()
{
    return appDir() + "/pets";
}

QString configDir()
{
    return dataRootDir() + "/config";
}

QString petsDir()
{
    return dataRootDir() + "/pets";
}

QString actionsDir()
{
    return dataRootDir() + "/actions";
}

QString logsDir()
{
    return dataRootDir() + "/logs";
}

QString chatLogsDir()
{
    return logsDir() + "/chat";
}

QString petLibraryFile()
{
    return dataRootDir() + "/petlibrary.json";
}

QString resizeToolPath()
{
#ifdef Q_OS_WIN
    return appDir() + "/DesktopPet-resize.exe";
#else
    return appDir() + "/DesktopPet-resize";
#endif
}

} // namespace AppPaths
