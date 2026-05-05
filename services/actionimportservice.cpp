#include "actionimportservice.h"

#include "core/gifframeextractor.h"
#include "core/petaction.h"
#include "core/petconfigmanager.h"
#include "core/petplaylist.h"

#include <QDir>
#include <QFile>

ActionImportResult ActionImportService::registerExistingAction(
    const QString &petDir,
    const PetBasicInfo &petInfo,
    const QList<PetAction> &currentActions,
    const PetPlaylist &currentPlaylist,
    const QString &actionId,
    const QString &folderPath,
    int fps,
    TargetCategory targetCategory,
    int timedIntervalSeconds,
    const QString &emotionName,
    TimedTriggerMode timedTriggerMode,
    const QString &triggerTime)
{
    ActionImportResult result;
    result.success = false;
    result.message = QString();

    for (const PetAction &existing : currentActions) {
        if (existing.id == actionId) {
            result.message = QObject::tr("动作 ID 已存在，请使用其他 ID。");
            return result;
        }
    }

    PetAction newAction;
    newAction.id = actionId;
    newAction.name = actionId;

    QDir petDirObj(petDir);
    QString relativePath = petDirObj.relativeFilePath(folderPath);
    newAction.folderPath = relativePath;

    newAction.fps = fps;

    QStringList frameFiles;
    QDir actionDir(folderPath);
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.webp";
    actionDir.setNameFilters(filters);
    actionDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    frameFiles = actionDir.entryList();
    newAction.frameCount = frameFiles.size();

    QList<PetAction> updatedActions = currentActions;
    updatedActions.append(newAction);

    QString petJsonPath = petDir + "/pet.json";
    if (!PetConfigManager::savePetJson(petJsonPath, petInfo, updatedActions)) {
        result.message = QObject::tr("保存 pet.json 失败。");
        return result;
    }

    PetPlaylist updatedPlaylist = currentPlaylist;
    if (targetCategory != TargetCategory::None) {
        ActionImportResult categoryResult = addToCategory(
            petDir, updatedPlaylist, actionId, targetCategory, timedIntervalSeconds, emotionName, timedTriggerMode, triggerTime);
        if (!categoryResult.success) {
            result.success = true;
            result.warning = true;
            result.message = QObject::tr("动作已添加，但加入分类失败，请稍后在动作设置中手动添加。");
            return result;
        }
    }

    result.success = true;
    result.message = QObject::tr("动作添加成功。");
    return result;
}

ActionImportResult ActionImportService::importGifAction(
    const QString &petDir,
    const PetBasicInfo &petInfo,
    const QList<PetAction> &currentActions,
    const PetPlaylist &currentPlaylist,
    const QString &gifPath,
    const QString &actionId,
    int fps,
    TargetCategory targetCategory,
    int timedIntervalSeconds,
    const QString &emotionName,
    TimedTriggerMode timedTriggerMode,
    const QString &triggerTime)
{
    ActionImportResult result;
    result.success = false;
    result.message = QString();

    for (const PetAction &existing : currentActions) {
        if (existing.id == actionId) {
            result.message = QObject::tr("动作 ID 已存在，请使用其他 ID。");
            return result;
        }
    }

    QString actionsDir = petDir + "/actions";
    QString actionDir = actionsDir + "/" + actionId;

    if (QDir(actionDir).exists()) {
        result.message = QObject::tr("动作目录已存在，请使用其他 ID。");
        return result;
    }

    GifExtractResult extractResult = GifFrameExtractor::extractGifToFrames(gifPath, actionDir);
    if (!extractResult.success) {
        QDir(actionDir).removeRecursively();
        result.message = extractResult.errorMessage;
        return result;
    }

    PetAction newAction;
    newAction.id = actionId;
    newAction.name = actionId;
    newAction.folderPath = "actions/" + actionId;
    newAction.fps = fps;
    newAction.frameCount = extractResult.frameCount;

    QList<PetAction> updatedActions = currentActions;
    updatedActions.append(newAction);

    QString petJsonPath = petDir + "/pet.json";
    if (!PetConfigManager::savePetJson(petJsonPath, petInfo, updatedActions)) {
        QDir(actionDir).removeRecursively();
        result.message = QObject::tr("保存 pet.json 失败。");
        return result;
    }

    PetPlaylist updatedPlaylist = currentPlaylist;
    if (targetCategory != TargetCategory::None) {
        ActionImportResult categoryResult = addToCategory(
            petDir, updatedPlaylist, actionId, targetCategory, timedIntervalSeconds, emotionName, timedTriggerMode, triggerTime);
        if (!categoryResult.success) {
            result.success = true;
            result.warning = true;
            result.message = QObject::tr("动作已添加，但加入分类失败，请稍后在动作设置中手动添加。");
            return result;
        }
    }

    result.success = true;
    result.message = QObject::tr("GIF 导入成功。");
    return result;
}

ActionImportResult ActionImportService::addToCategory(
    const QString &petDir,
    PetPlaylist &playlist,
    const QString &actionId,
    TargetCategory targetCategory,
    int timedIntervalSeconds,
    const QString &emotionName,
    TimedTriggerMode timedTriggerMode,
    const QString &triggerTime)
{
    ActionImportResult result;
    result.success = false;
    result.message = QString();

    PetActionRef ref(actionId);
    ref.loop = false;
    ref.repeat = 1;
    ref.animationSpeed = 1.0;
    ref.moveEnabled = false;
    ref.movementSpeed = 1.0;

    if (targetCategory == TargetCategory::Timed) {
        ref.intervalSeconds = timedIntervalSeconds;
        ref.timedTriggerMode = timedTriggerMode;
        ref.triggerTime = triggerTime;
    } else if (targetCategory == TargetCategory::Emotion) {
        ref.emotion = emotionName;
    }

    switch (targetCategory) {
        case TargetCategory::Idle:
            playlist.addIdleAction(ref);
            break;
        case TargetCategory::Random:
            playlist.addRandomAction(ref);
            break;
        case TargetCategory::Timed:
            playlist.addTimedAction(ref);
            break;
        case TargetCategory::Emotion:
            playlist.addEmotionAction(emotionName, ref);
            break;
        default:
            break;
    }

    QString playlistPath = QDir(petDir).filePath("playlist.json");
    if (!PetConfigManager::savePlaylistToJson(playlistPath, playlist)) {
        result.message = QObject::tr("保存 playlist.json 失败。");
        return result;
    }

    result.success = true;
    return result;
}
