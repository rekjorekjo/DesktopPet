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
    static QString defaultPetDirectory();
    static QString legacyDefaultPetDirectory();
    static QString currentPetJsonPath();
    static QString currentPlaylistPath();
    static QString defaultPetJsonPath();
    static QString defaultPlaylistPath();

    static QString resolveActionDirectory(const PetAction &action);

    static bool ensureDefaultStructure();

private:
    static bool migrateFromLegacyStructure();
    static bool copyDirectory(const QString &source, const QString &destination);
};

#endif // PETPATHS_H
