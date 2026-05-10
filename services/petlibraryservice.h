#ifndef PETLIBRARYSERVICE_H
#define PETLIBRARYSERVICE_H

#include "services/petlibraryindexservice.h"
#include <QString>

struct PetLibraryOperationResult
{
    bool success = false;
    bool warning = false;
    QString message;
    QString nextCurrentPetId;
};

class PetLibraryService
{
public:
    static PetLibraryOperationResult disablePet(const QString &petId);
    static PetLibraryOperationResult deletePet(const QString &petId);

private:
    static QString findFirstEnabledPetId(const QString &excludePetId);
    static int countEnabledPets(const QString &excludePetId = QString());
};

#endif
