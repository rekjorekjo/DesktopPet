#include "petlibraryservice.h"

#include "core/appsettings.h"
#include "core/petpaths.h"
#include "services/petlibraryindexservice.h"

#include <QDir>
#include <QFile>

PetLibraryOperationResult PetLibraryService::disablePet(const QString &petId)
{
    PetLibraryOperationResult result;
    result.success = false;

    if (petId.isEmpty()) {
        result.message = QObject::tr("宠物 ID 不能为空。");
        return result;
    }

    PetLibraryIndexService::ensureLibrary();

    auto entry = PetLibraryIndexService::findPet(petId);
    if (!entry.has_value()) {
        result.success = true;
        result.message = QObject::tr("宠物不在库中。");
        return result;
    }

    QString petName = entry->name.isEmpty() ? petId : entry->name;

    if (!PetLibraryIndexService::removePetEntry(petId)) {
        result.message = QObject::tr("从宠物库移除记录失败。");
        return result;
    }

    result.success = true;
    result.message = QObject::tr("宠物 \"%1\" 已从列表移除。").arg(petName);

    QString currentPetId = AppSettings::currentPetId();
    if (petId == currentPetId) {
        result.nextCurrentPetId = PetLibraryIndexService::findFirstEnabledPetId(petId);
    }

    return result;
}

PetLibraryOperationResult PetLibraryService::deletePet(const QString &petId)
{
    PetLibraryOperationResult result;
    result.success = false;

    if (petId.isEmpty()) {
        result.message = QObject::tr("宠物 ID 不能为空。");
        return result;
    }

    PetLibraryIndexService::ensureLibrary();

    auto entry = PetLibraryIndexService::findPet(petId);
    QString petName = entry.has_value() && !entry->name.isEmpty() ? entry->name : petId;

    QString currentPetId = AppSettings::currentPetId();
    if (petId == currentPetId) {
        result.nextCurrentPetId = PetLibraryIndexService::findFirstEnabledPetId(petId);
    }

    if (!PetLibraryIndexService::deletePetEntryAndDirectory(petId)) {
        result.message = QObject::tr("删除宠物失败。");
        return result;
    }

    result.success = true;
    result.message = QObject::tr("宠物 \"%1\" 已删除。").arg(petName);

    return result;
}

QString PetLibraryService::findFirstEnabledPetId(const QString &excludePetId)
{
    return PetLibraryIndexService::findFirstEnabledPetId(excludePetId);
}

int PetLibraryService::countEnabledPets(const QString &excludePetId)
{
    QList<PetLibraryEntry> entries = PetLibraryIndexService::loadEntries();

    int count = 0;
    for (const PetLibraryEntry &entry : entries) {
        if (entry.id == excludePetId) {
            continue;
        }
        ++count;
    }

    return count;
}
