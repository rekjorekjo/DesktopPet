#include "petlibraryservice.h"

#include "core/appsettings.h"
#include "core/petconfigmanager.h"
#include "core/petpaths.h"

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

    QString petDir = PetPaths::petDirectory(petId);
    QString petJsonPath = petDir + "/pet.json";

    QDir dir(petDir);
    if (!dir.exists()) {
        QString currentPetId = AppSettings::currentPetId();
        if (petId == currentPetId) {
            result.nextCurrentPetId = findFirstEnabledPetId(petId);
        }
        result.success = true;
        result.message = QObject::tr("宠物目录不存在，已移除引用。");
        return result;
    }

    PetBasicInfo info;
    bool petJsonLoaded = PetConfigManager::loadPetInfoJson(petJsonPath, info);

    if (petJsonLoaded) {
        info.enabled = false;
    } else {
        info.id = petId;
        info.name = petId;
        info.enabled = false;
        info.canvasSize = QSize(400, 400);
        info.displaySize = QSize(200, 200);
    }

    if (!PetConfigManager::savePetInfoJson(petJsonPath, info)) {
        result.message = QObject::tr("保存宠物配置失败。");
        return result;
    }

    result.success = true;
    result.message = QObject::tr("宠物 \"%1\" 已从列表移除。").arg(info.name.isEmpty() ? petId : info.name);

    QString currentPetId = AppSettings::currentPetId();
    if (petId == currentPetId) {
        result.nextCurrentPetId = findFirstEnabledPetId(petId);
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

    QString petDir = PetPaths::petDirectory(petId);
    QDir dir(petDir);

    QString canonicalPetDir = dir.canonicalPath();

    if (canonicalPetDir.isEmpty()) {
        QString currentPetId = AppSettings::currentPetId();
        if (petId == currentPetId) {
            result.nextCurrentPetId = findFirstEnabledPetId(petId);
        }
        result.success = true;
        result.message = QObject::tr("宠物目录不存在，已移除引用。");
        return result;
    }

    QString petsDir = QDir(PetPaths::petsDirectory()).canonicalPath();
    if (petsDir.isEmpty()) {
        QString currentPetId = AppSettings::currentPetId();
        if (petId == currentPetId) {
            result.nextCurrentPetId = findFirstEnabledPetId(petId);
        }
        result.success = true;
        result.message = QObject::tr("宠物根目录不存在，已移除引用。");
        return result;
    }

    if (!canonicalPetDir.startsWith(petsDir + "/")) {
        result.message = QObject::tr("只能删除宠物目录下的宠物。");
        return result;
    }

    QString currentPetId = AppSettings::currentPetId();
    if (petId == currentPetId) {
        result.nextCurrentPetId = findFirstEnabledPetId(petId);
    }

    if (!dir.removeRecursively()) {
        result.message = QObject::tr("删除宠物目录失败。");
        return result;
    }

    result.success = true;
    result.message = QObject::tr("宠物 \"%1\" 已删除。").arg(petId);

    return result;
}

QString PetLibraryService::findFirstEnabledPetId(const QString &excludePetId)
{
    QDir petsDir(PetPaths::petsDirectory());
    if (!petsDir.exists()) {
        return QString();
    }

    QStringList petFolders = petsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &petId : petFolders) {
        if (petId == excludePetId) {
            continue;
        }

        QString petJsonPath = PetPaths::petDirectory(petId) + "/pet.json";
        PetBasicInfo info;
        if (PetConfigManager::loadPetInfoJson(petJsonPath, info) && info.enabled) {
            return petId;
        }
    }

    return QString();
}

int PetLibraryService::countEnabledPets(const QString &excludePetId)
{
    QDir petsDir(PetPaths::petsDirectory());
    if (!petsDir.exists()) {
        return 0;
    }

    int count = 0;
    QStringList petFolders = petsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &petId : petFolders) {
        if (petId == excludePetId) {
            continue;
        }

        QString petJsonPath = PetPaths::petDirectory(petId) + "/pet.json";
        PetBasicInfo info;
        if (PetConfigManager::loadPetInfoJson(petJsonPath, info) && info.enabled) {
            ++count;
        }
    }

    return count;
}
