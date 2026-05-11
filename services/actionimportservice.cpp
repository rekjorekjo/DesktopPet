#include "actionimportservice.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "models/petaction.h"
#include "models/petplaylist.h"
#include "services/actionlibraryindexservice.h"
#include "utils/gifframeextractor.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

namespace {
bool isValidActionId(const QString &actionId)
{
    if (actionId.isEmpty()) {
        return false;
    }
    if (actionId == "." || actionId == "..") {
        return false;
    }
    if (actionId.contains("/") || actionId.contains("\\")) {
        return false;
    }
    static QRegularExpression re("^[a-zA-Z0-9_-]+$");
    return re.match(actionId).hasMatch();
}

bool resetActionDirectoryForImport(const QString &actionId, QString *errorMessage)
{
    if (!isValidActionId(actionId)) {
        if (errorMessage) {
            *errorMessage = QObject::tr("动作 ID 格式无效。");
        }
        return false;
    }

    QString actionsBaseDir = PetPaths::actionsDirectory();
    QString targetDirPath = QDir(actionsBaseDir).filePath(actionId);
    QDir targetDir(targetDirPath);

    if (!targetDir.exists()) {
        if (!QDir().mkpath(targetDirPath)) {
            if (errorMessage) {
                *errorMessage = QObject::tr("创建动作目录失败。");
            }
            return false;
        }
        return true;
    }

    QString canonicalActionsDir = QDir(actionsBaseDir).canonicalPath();
    QString canonicalTargetDir = targetDir.canonicalPath();

    if (canonicalActionsDir.isEmpty() || canonicalTargetDir.isEmpty()) {
        if (errorMessage) {
            *errorMessage = QObject::tr("无法验证动作目录路径。");
        }
        return false;
    }

#ifdef Q_OS_WIN
    QString actionsDirCmp = canonicalActionsDir.toLower();
    QString targetDirCmp = canonicalTargetDir.toLower();
#else
    QString actionsDirCmp = canonicalActionsDir;
    QString targetDirCmp = canonicalTargetDir;
#endif

    if (targetDirCmp == actionsDirCmp) {
        if (errorMessage) {
            *errorMessage = QObject::tr("不允许操作动作库根目录。");
        }
        return false;
    }

    if (!targetDirCmp.startsWith(actionsDirCmp + "/")) {
        if (errorMessage) {
            *errorMessage = QObject::tr("动作目录不在动作库内。");
        }
        return false;
    }

    if (!targetDir.removeRecursively()) {
        if (errorMessage) {
            *errorMessage = QObject::tr("清理旧动作目录失败。");
        }
        return false;
    }

    if (!QDir().mkpath(targetDirPath)) {
        if (errorMessage) {
            *errorMessage = QObject::tr("重新创建动作目录失败。");
        }
        return false;
    }

    return true;
}
}

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

    if (!isValidActionId(actionId)) {
        result.message = QObject::tr("动作 ID 格式无效，只能包含字母、数字、下划线和短横线。");
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

    if (!isValidActionId(actionId)) {
        result.message = QObject::tr("动作 ID 格式无效，只能包含字母、数字、下划线和短横线。");
        return result;
    }

    ActionLibraryIndexService::ensureLibrary();

    if (ActionLibraryIndexService::containsActionId(actionId)) {
        result.message = QObject::tr("动作 ID 已存在于动作库。请从动作库添加到当前分类，或使用其他 ID。");
        return result;
    }

    QDir sourceDir(folderPath);
    if (!sourceDir.exists()) {
        result.message = QObject::tr("源动作文件夹不存在。");
        return result;
    }

    const QStringList frameFiles = PetConfigManager::scanFrameFiles(folderPath);
    if (frameFiles.isEmpty()) {
        result.message = QObject::tr("源动作文件夹中没有图片帧。");
        return result;
    }

    QString resetError;
    if (!resetActionDirectoryForImport(actionId, &resetError)) {
        result.message = resetError;
        return result;
    }

    QDir actionsDir(PetPaths::actionsDirectory());
    QString targetActionDir = actionsDir.filePath(actionId);
    QDir targetDir(targetActionDir);

    bool copySuccess = true;
    for (const QString &srcPath : frameFiles) {
        const QString destPath = targetDir.filePath(QFileInfo(srcPath).fileName());
        if (!QFile::copy(srcPath, destPath)) {
            copySuccess = false;
            break;
        }
    }

    if (!copySuccess) {
        result.message = QObject::tr("复制动作帧失败。");
        return result;
    }

    PetAction action = PetConfigManager::loadGlobalActionFromDirectory(actionId, targetActionDir);
    if (!action.isValid()) {
        result.message = QObject::tr("读取动作帧失败。");
        return result;
    }
    action.fps = fps;

    if (!PetConfigManager::saveActionMetadata(targetActionDir, action)) {
        result.message = QObject::tr("保存动作元数据失败。");
        return result;
    }

    ActionLibraryEntry entry;
    entry.id = actionId;
    entry.name = action.name.isEmpty() ? actionId : action.name;
    entry.dir = actionId;
    ActionLibraryIndexService::addOrUpdateAction(entry);

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

    if (!isValidActionId(actionId)) {
        result.message = QObject::tr("动作 ID 格式无效，只能包含字母、数字、下划线和短横线。");
        return result;
    }

    ActionLibraryIndexService::ensureLibrary();

    if (ActionLibraryIndexService::containsActionId(actionId)) {
        result.message = QObject::tr("动作 ID 已存在于动作库。请从动作库添加到当前分类，或使用其他 ID。");
        return result;
    }

    QFileInfo gifInfo(gifPath);
    if (!gifInfo.exists() || !gifInfo.isFile()) {
        result.message = QObject::tr("GIF 文件不存在。");
        return result;
    }

    QString resetError;
    if (!resetActionDirectoryForImport(actionId, &resetError)) {
        result.message = resetError;
        return result;
    }

    QDir actionsDir(PetPaths::actionsDirectory());
    QString actionDir = actionsDir.filePath(actionId);

    GifExtractResult extractResult = GifFrameExtractor::extractGifToFrames(gifPath, actionDir);
    if (!extractResult.success) {
        result.message = extractResult.errorMessage;
        return result;
    }

    PetAction action = PetConfigManager::loadGlobalActionFromDirectory(actionId, actionDir);
    if (!action.isValid()) {
        result.message = QObject::tr("读取动作帧失败。");
        return result;
    }
    action.fps = fps;

    if (!PetConfigManager::saveActionMetadata(actionDir, action)) {
        result.message = QObject::tr("保存动作元数据失败。");
        return result;
    }

    ActionLibraryEntry entry;
    entry.id = actionId;
    entry.name = action.name.isEmpty() ? actionId : action.name;
    entry.dir = actionId;
    ActionLibraryIndexService::addOrUpdateAction(entry);

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

    QString baseDisplayName = actionId;

    auto entry = ActionLibraryIndexService::findAction(actionId);
    if (entry.has_value() && !entry->name.isEmpty()) {
        baseDisplayName = entry->name;
    }

    QList<PetActionRef> existingRefs;
    switch (targetCategory) {
        case TargetCategory::Idle:
            existingRefs = playlist.idleActions();
            break;
        case TargetCategory::Random:
            existingRefs = playlist.randomActions();
            break;
        case TargetCategory::Timed:
            existingRefs = playlist.timedActions();
            break;
        case TargetCategory::Emotion:
            existingRefs = playlist.emotionActions(emotionName);
            break;
        default:
            break;
    }

    QString uniqueDisplayName = baseDisplayName;
    auto isDuplicate = [&](const QString &displayName) -> bool {
        for (const PetActionRef &ref : existingRefs) {
            if (ref.actionId == actionId && ref.displayName.trimmed() == displayName.trimmed()) {
                return true;
            }
        }
        return false;
    };

    if (isDuplicate(baseDisplayName)) {
        int suffix = 2;
        while (suffix <= 1000) {
            QString candidate = baseDisplayName + " " + QString::number(suffix);
            if (!isDuplicate(candidate)) {
                uniqueDisplayName = candidate;
                break;
            }
            ++suffix;
        }
    }

    PetActionRef ref(actionId);
    ref.displayName = uniqueDisplayName;
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
