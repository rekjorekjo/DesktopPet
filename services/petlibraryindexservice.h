#ifndef PETLIBRARYINDEXSERVICE_H
#define PETLIBRARYINDEXSERVICE_H

#include <QList>
#include <QString>
#include <optional>

struct PetLibraryEntry
{
    QString id;
    QString name;
    QString dir;
    bool enabled = true;
};

class PetLibraryIndexService
{
public:
    static QString libraryFilePath();

    static bool ensureLibrary();

    static QList<PetLibraryEntry> loadEntries();
    static bool saveEntries(const QList<PetLibraryEntry> &entries);

    static bool containsPetId(const QString &petId);
    static std::optional<PetLibraryEntry> findPet(const QString &petId);

    static bool addOrUpdatePet(const PetLibraryEntry &entry);
    static bool removePetEntry(const QString &petId);
    static bool deletePetEntryAndDirectory(const QString &petId);

    static QString findFirstEnabledPetId(const QString &excludePetId = QString());

private:
    static QList<PetLibraryEntry> scanExistingPets();
};

#endif // PETLIBRARYINDEXSERVICE_H
