#include "petcreationservice.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"

#include <QDir>
#include <QObject>
#include <QRegularExpression>

PetCreationResult PetCreationService::createPet(
    const QString &petId,
    const QString &petName,
    const QSize &canvasSize,
    const QSize &displaySize)
{
    PetCreationResult result;
    result.success = false;

    QString trimmedId = petId.trimmed();
    if (trimmedId.isEmpty()) {
        result.message = QObject::tr("宠物 ID 不能为空。");
        return result;
    }

    QRegularExpression idPattern("^[a-zA-Z0-9_-]+$");
    if (!idPattern.match(trimmedId).hasMatch()) {
        result.message = QObject::tr("宠物 ID 只能包含字母、数字、下划线和短横线。");
        return result;
    }

    QString targetDir = PetPaths::petDirectory(trimmedId);
    if (QDir(targetDir).exists()) {
        result.message = QObject::tr("宠物 ID 已存在，请使用其他 ID。");
        return result;
    }

    QDir dir;
    if (!dir.mkpath(targetDir)) {
        result.message = QObject::tr("创建宠物目录失败。");
        return result;
    }

    PetBasicInfo info;
    info.id = trimmedId;
    info.name = petName.trimmed().isEmpty() ? trimmedId : petName.trimmed();
    info.canvasSize = canvasSize.isValid() ? canvasSize : QSize(400, 400);
    info.displaySize = displaySize.isValid() ? displaySize : QSize(200, 200);

    PetPlaylist playlist;

    QDir actionsDir(PetPaths::actionsDirectory());
    QStringList actionFolders = actionsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    bool hasAction = false;
    for (const QString &actionFolder : actionFolders) {
        QString actionPath = PetPaths::actionsDirectory() + "/" + actionFolder;

        QStringList frameFiles = PetConfigManager::scanFrameFiles(actionPath);

        if (frameFiles.isEmpty()) {
            continue;
        }

        PetActionRef ref;
        ref.actionId = actionFolder;
        ref.loop = true;
        ref.repeat = 0;
        ref.animationSpeed = 1.0;
        ref.moveEnabled = false;
        ref.movementSpeed = 1.0;
        ref.timedTriggerMode = TimedTriggerMode::Interval;
        ref.triggerTime = "00:00";

        playlist.addIdleAction(ref);
        hasAction = true;
        break;
    }

    QString petJsonPath = targetDir + "/pet.json";
    if (!PetConfigManager::savePetInfoJson(petJsonPath, info)) {
        result.message = QObject::tr("保存 pet.json 失败。");
        return result;
    }

    QString playlistPath = targetDir + "/playlist.json";
    if (!PetConfigManager::savePlaylistToJson(playlistPath, playlist)) {
        result.message = QObject::tr("保存 playlist.json 失败。");
        return result;
    }

    result.success = true;
    result.petId = trimmedId;

    if (!hasAction) {
        result.warning = true;
        result.message = QObject::tr("宠物已创建，但动作库为空，请前往设置新增动作。");
    } else {
        result.message = QObject::tr("宠物 %1 创建成功。").arg(info.name);
    }

    return result;
}
