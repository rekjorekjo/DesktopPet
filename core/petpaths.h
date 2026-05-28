#ifndef PETPATHS_H
#define PETPATHS_H

#include <QString>

struct PetAction;

class PetPaths
{
public:
    static QString rootDirectory();
    static QString actionsDirectory();
    static QString petsDirectory();
    static QString petDirectory(const QString &petId);
    static QString currentPetDirectory();
    static QString currentPetJsonPath();
    static QString currentPlaylistPath();

    static QString resolveActionDirectory(const PetAction &action);

    static QString configDir();
    static QString apiProfilesFilePath();
    static QString chatSettingsFilePath();
    static QString webSearchSettingsFilePath();

    static bool ensureDefaultStructure();

    // Test-only: override the data root directory for isolated testing.
    // Call clearDataRootOverrideForTesting() in cleanup.
    static void setDataRootOverrideForTesting(const QString &path);
    static void clearDataRootOverrideForTesting();
};

#endif // PETPATHS_H
