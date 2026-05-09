#ifndef PETCREATIONSERVICE_H
#define PETCREATIONSERVICE_H

#include <QSize>
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
    static PetCreationResult createPet(
        const QString &petId,
        const QString &petName,
        const QSize &canvasSize = QSize(400, 400),
        const QSize &displaySize = QSize(200, 200)
    );

    static PetCreationResult createOrRepairPetConfig(
        const QString &petId,
        const QString &petName,
        const QSize &canvasSize = QSize(400, 400),
        const QSize &displaySize = QSize(200, 200)
    );
};

#endif // PETCREATIONSERVICE_H
