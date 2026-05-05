#include "actionimportservice.h"

#include "core/gifframeextractor.h"
#include "core/petaction.h"
#include "core/petconfigmanager.h"
#include "core/petpaths.h"
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
    Q_UNUSED(petDir);

    ActionImportResult result;
    result.success = false;
    result.message = QString();

    for (const PetAction &existing : currentActions) {
        if (existing.id == actionId) {
            result.message = QObject::tr("动作 ID 已存在，请使用其他 ID。");
            return result;
        }
    }

    QString targetActionDir = PetPaths::actionsDirectory() + "/" + actionId;
    if (QDir(targetActionDir).exists()) {
        result.message = QObject::tr("动作目录已存在，请使用其他 ID。");
        return result;
    }

    QDir targetDir(targetActionDir);
    if (!targetDir.mkpath(".")) {
        result.message = QObject::tr("创建动作目录失败。");
        return result;
    }

    QDir sourceDir(folderPath);
    if (!sourceDir.exists()) {
        result.message = QObject::tr("源动作文件夹不存在。");
        return result;
    }

    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.jpeg" << "*.webp";
    sourceDir.setNameFilters(filters);
    sourceDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    QStringList frameFiles = sourceDir.entryList();

    bool copySuccess = true;
    for (const QString &frameFile : frameFiles) {
        QString srcPath = folderPath + "/" + frameFile;
        QString destPath = targetActionDir + "/" + frameFile;
        if (!QFile::copy(srcPath, destPath)) {
            copySuccess = false;
            break;
        }
    }

    if (!copySuccess) {
        targetDir.removeRecursively();
        result.message = QObject::tr("复制动作帧失败。");
        return result;
    }

    PetAction newAction;
    newAction.id = actionId;
    newAction.name = actionId;
    newAction.folderPath = actionId;
    newAction.fps = fps;
    newAction.frameCount = frameFiles.size();
    newAction.enabled = true;

    QList<PetAction> updatedActions = currentActions;
    updatedActions.append(newAction);

    QString petJsonPath = PetPaths::defaultPetJsonPath();
    if (!PetConfigManager::savePetJson(petJsonPath, petInfo, updatedActions)) {
        targetDir.removeRecursively();
        result.message = QObject::tr("保存 pet.json 失败。");
        return result;
    }

    PetPlaylist updatedPlaylist = currentPlaylist;
    if (targetCategory != TargetCategory::None) {
        ActionImportResult categoryResult = addToCategory(
            PetPaths::defaultPetDirectory(), updatedPlaylist, actionId, targetCategory, timedIntervalSeconds, emotionName, timedTriggerMode, triggerTime);
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
    Q_UNUSED(petDir);

    ActionImportResult result;
    result.success = false;
    result.message = QString();

    for (const PetAction &existing : currentActions) {
        if (existing.id == actionId) {
            result.message = QObject::tr("动作 ID 已存在，请使用其他 ID。");
            return result;
        }
    }

    QString actionsDir = PetPaths::actionsDirectory();
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
    newAction.folderPath = actionId;
    newAction.fps = fps;
    newAction.frameCount = extractResult.frameCount;
    newAction.enabled = true;

    QList<PetAction> updatedActions = currentActions;
    updatedActions.append(newAction);

    QString petJsonPath = PetPaths::defaultPetJsonPath();
    if (!PetConfigManager::savePetJson(petJsonPath, petInfo, updatedActions)) {
        QDir(actionDir).removeRecursively();
        result.message = QObject::tr("保存 pet.json 失败。");
        return result;
    }

    PetPlaylist updatedPlaylist = currentPlaylist;
    if (targetCategory != TargetCategory::None) {
        ActionImportResult categoryResult = addToCategory(
            PetPaths::defaultPetDirectory(), updatedPlaylist, actionId, targetCategory, timedIntervalSeconds, emotionName, timedTriggerMode, triggerTime);
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
