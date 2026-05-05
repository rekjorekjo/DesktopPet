#ifndef PETCREATIONSERVICE_H
#define PETCREATIONSERVICE_H

#include <QString>

struct PetCreationResult
{
    bool success = false;
    bool warning = false;
    QString petId;
    QString message;
};

class PetCreationService
{
public:
    static PetCreationResult createPet(const QString &petId, const QString &petName);
};

#endif // PETCREATIONSERVICE_H
