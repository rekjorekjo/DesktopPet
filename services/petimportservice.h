#ifndef PETIMPORTSERVICE_H
#define PETIMPORTSERVICE_H

#include <QSize>
#include <QString>

struct PetImportResult
{
    bool success = false;
    bool warning = false;
    QString petId;
    QString message;
};

class PetImportService
{
public:
    static PetImportResult importPet(
        const QString &sourceDir,
        const QString &targetPetId,
        const QString &petName,
        const QSize &canvasSize,
        const QSize &displaySize
    );
};

#endif
