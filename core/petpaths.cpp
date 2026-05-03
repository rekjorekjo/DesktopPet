#include "petpaths.h"

#include <QDir>

QString PetPaths::defaultPetDirectory()
{
    return QDir::currentPath() + "/pets/default_pet";
}
