#include "actionimportservice.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "models/petaction.h"
#include "models/petplaylist.h"
#include "utils/gifframeextractor.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

ActionImportResult ActionImportService::registerGlobalActionToPet(
    const QString &petDir,
    const PetBasicInfo &petInfo,
    QList<PetAction> currentActions,
    PetPlaylist currentPlaylist,
    const QString &actionId,
    int fps,
    TargetCategory targetCategory,
    int timedIntervalSeconds,
    const QString &emotionName,
    TimedTriggerMode timedTriggerMode,
    const QString &triggerTime)
{
    Q_UNUSED(petInfo);
    Q_UNUSED(currentActions);
    Q_UNUSED(fps);

    ActionImportResult result;
    result.success = false;
    result.message = QString();

    if (actionId.isEmpty()) {
        result.message = QObject::tr("动作 ID 不能为空。");
        return result;
    }

    QDir actionsDir(PetPaths::actionsDirectory());
    QString actionDir = actionsDir.filePath(actionId);
    if (!QDir(actionDir).exists()) {
        result.message = QObject::tr("动作资源目录不存在。");
        return result;
    }

    const QStringList frameFiles = PetConfigManager::scanFrameFiles(actionDir);

    if (frameFiles.isEmpty()) {
        result.message = QObject::tr("动作目录中没有图片帧。");
        return result;
    }

    if (targetCategory != TargetCategory::None) {
        ActionImportResult categoryResult = addToCategory(
            petDir, currentPlaylist, actionId, targetCategory, timedIntervalSeconds, emotionName, timedTriggerMode, triggerTime);
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
    Q_UNUSED(petInfo);
    Q_UNUSED(currentActions);

    ActionImportResult result;
    result.success = false;
    result.message = QString();

    QDir actionsDir(PetPaths::actionsDirectory());
    QString targetActionDir = actionsDir.filePath(actionId);
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
        targetDir.removeRecursively();
        result.message = QObject::tr("源动作文件夹不存在。");
        return result;
    }

    const QStringList frameFiles = PetConfigManager::scanFrameFiles(folderPath);
    if (frameFiles.isEmpty()) {
        targetDir.removeRecursively();
        result.message = QObject::tr("源动作文件夹中没有图片帧。");
        return result;
    }

    bool copySuccess = true;
    for (const QString &srcPath : frameFiles) {
        const QString destPath = targetDir.filePath(QFileInfo(srcPath).fileName());
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

    PetAction action = PetConfigManager::loadGlobalActionFromDirectory(actionId, targetActionDir);
    if (!action.isValid()) {
        targetDir.removeRecursively();
        result.message = QObject::tr("读取动作帧失败。");
        return result;
    }
    action.fps = fps;

    if (!PetConfigManager::saveActionMetadata(targetActionDir, action)) {
        targetDir.removeRecursively();
        result.message = QObject::tr("保存动作元数据失败。");
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
    Q_UNUSED(petInfo);
    Q_UNUSED(currentActions);

    ActionImportResult result;
    result.success = false;
    result.message = QString();

    QDir actionsDir(PetPaths::actionsDirectory());
    QString actionDir = actionsDir.filePath(actionId);

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

    PetAction action = PetConfigManager::loadGlobalActionFromDirectory(actionId, actionDir);
    if (!action.isValid()) {
        QDir(actionDir).removeRecursively();
        result.message = QObject::tr("读取动作帧失败。");
        return result;
    }
    action.fps = fps;

    if (!PetConfigManager::saveActionMetadata(actionDir, action)) {
        QDir(actionDir).removeRecursively();
        result.message = QObject::tr("保存动作元数据失败。");
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
