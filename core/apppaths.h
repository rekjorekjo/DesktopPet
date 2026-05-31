#ifndef APPPATHS_H
#define APPPATHS_H

#include <QString>

// Unified path utilities for DesktopPet.
// All core data paths are based on the application executable directory,
// not QDir::currentPath(), to ensure consistent behavior regardless of
// how the application is launched (shortcut, terminal, IDE, etc).
//
// Current data layout uses appDir/pets as the data root (historical convention).
// PetPaths delegates to AppPaths and adds test-override support.

namespace AppPaths {

// Returns QCoreApplication::applicationDirPath().
QString appDir();

// DesktopPet runtime data root.
// Current version preserves historical layout: appDir/pets
QString dataRootDir();

// dataRootDir/config
QString configDir();

// dataRootDir/pets
QString petsDir();

// dataRootDir/actions
QString actionsDir();

// dataRootDir/logs
QString logsDir();

// dataRootDir/logs/chat
QString chatLogsDir();

// dataRootDir/petlibrary.json
QString petLibraryFile();

// appDir/DesktopPet-resize.exe (or DesktopPet-resize on non-Windows)
QString resizeToolPath();

} // namespace AppPaths

#endif // APPPATHS_H
