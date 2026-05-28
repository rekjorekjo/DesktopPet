#include "petimportservice.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "services/petlibraryindexservice.h"

#include <QDir>
#include <QFile>
#include <QRegularExpression>

PetImportResult PetImportService::importPet(
    const QString &sourceDir,
    const QString &targetPetId,
    const QString &petName,
    const QSize &canvasSize,
    const QSize &displaySize)
{
    PetImportResult result;
    result.success = false;

    if (sourceDir.isEmpty()) {
        result.message = QObject::tr("源目录不能为空。");
        return result;
    }

    QDir sourceDirectory(sourceDir);
    if (!sourceDirectory.exists()) {
        result.message = QObject::tr("源目录不存在。");
        return result;
    }

    QString sourcePetJson = sourceDir + "/pet.json";
    QString sourcePlaylist = sourceDir + "/playlist.json";

    if (!QFile::exists(sourcePetJson)) {
        result.message = QObject::tr("源目录缺少 pet.json 文件。");
        return result;
    }

    if (!QFile::exists(sourcePlaylist)) {
        result.message = QObject::tr("源目录缺少 playlist.json 文件。");
        return result;
    }

    if (targetPetId.isEmpty()) {
        result.message = QObject::tr("宠物 ID 不能为空。");
        return result;
    }

    static QRegularExpression idPattern("^[a-zA-Z0-9_-]+$");
    if (!idPattern.match(targetPetId).hasMatch()) {
        result.message = QObject::tr("宠物 ID 只能包含字母、数字、下划线和短横线。");
        return result;
    }

    PetLibraryIndexService::ensureLibrary();

    if (PetLibraryIndexService::containsPetId(targetPetId)) {
        result.message = QObject::tr("宠物 ID 已存在，请使用其他 ID。");
        return result;
    }

    QString targetDir = PetPaths::petDirectory(targetPetId);

    if (!QDir(targetDir).exists()) {
        if (!QDir().mkpath(targetDir)) {
            result.message = QObject::tr("创建目标目录失败。");
            return result;
        }
    }

    QString targetPetJson = targetDir + "/pet.json";
    QString targetPlaylist = targetDir + "/playlist.json";

    if (QFile::exists(targetPetJson)) {
        QFile::remove(targetPetJson);
    }
    if (QFile::exists(targetPlaylist)) {
        QFile::remove(targetPlaylist);
    }

    if (!QFile::copy(sourcePetJson, targetPetJson)) {
        result.message = QObject::tr("复制 pet.json 失败。");
        return result;
    }

    if (!QFile::copy(sourcePlaylist, targetPlaylist)) {
        QFile::remove(targetPetJson);
        result.message = QObject::tr("复制 playlist.json 失败。");
        return result;
    }

    PetBasicInfo info;
    if (!PetConfigManager::loadPetInfoJson(targetPetJson, info)) {
        QFile::remove(targetPetJson);
        QFile::remove(targetPlaylist);
        result.message = QObject::tr("读取宠物配置失败。");
        return result;
    }

    info.id = targetPetId;
    info.name = petName.isEmpty() ? targetPetId : petName;
    info.canvasSize = canvasSize.isValid() ? canvasSize : QSize(400, 400);
    info.displaySize = displaySize.isValid() ? displaySize : QSize(200, 200);
    info.enabled = true;

    if (!PetConfigManager::savePetInfoJson(targetPetJson, info)) {
        QFile::remove(targetPetJson);
        QFile::remove(targetPlaylist);
        result.message = QObject::tr("保存宠物配置失败。");
        return result;
    }

    PetLibraryEntry entry;
    entry.id = targetPetId;
    entry.name = info.name;
    entry.enabled = true;
    PetLibraryIndexService::addOrUpdatePet(entry);

    result.success = true;
    result.petId = targetPetId;
    result.message = QObject::tr("宠物 \"%1\" 导入成功。").arg(info.name);

    PetPlaylist playlist;
    if (PetConfigManager::loadPlaylistFromJson(targetPlaylist, playlist)) {
        bool hasMissingActions = false;
        QString actionsDir = PetPaths::actionsDirectory();
        QDir actionsDirectory(actionsDir);

        QStringList allActionIds;
        for (const PetActionRef &ref : playlist.idleActions()) {
            allActionIds.append(ref.actionId);
        }
        for (const PetActionRef &ref : playlist.randomActions()) {
            allActionIds.append(ref.actionId);
        }
        for (const PetActionRef &ref : playlist.timedActions()) {
            allActionIds.append(ref.actionId);
        }
        for (const QString &emotion : playlist.allEmotionActions().keys()) {
            for (const PetActionRef &ref : playlist.emotionActions(emotion)) {
                allActionIds.append(ref.actionId);
            }
        }

        for (const QString &actionId : allActionIds) {
            QString actionDir = actionsDirectory.filePath(actionId);
            if (!QDir(actionDir).exists()) {
                hasMissingActions = true;
                break;
            }
        }

        if (hasMissingActions) {
            result.warning = true;
            result.message = QObject::tr("宠物已导入，但部分动作资源可能不存在，请检查动作库。");
        }
    }

    return result;
}
