#include "actionimportworker.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "models/petaction.h"
#include "services/actionimportservice.h"
#include "services/actionlibraryindexservice.h"

#include <QDir>

ActionImportWorker::ActionImportWorker(const ImportFolderTask &task, QObject *parent)
    : QObject(parent)
    , m_taskType(TaskType::ImportFolder)
    , m_folderTask(task)
{
}

ActionImportWorker::ActionImportWorker(const ImportGifTask &task, QObject *parent)
    : QObject(parent)
    , m_taskType(TaskType::ImportGif)
    , m_gifTask(task)
{
}

ActionImportWorker::ActionImportWorker(const ImportBatchTask &task, QObject *parent)
    : QObject(parent)
    , m_taskType(TaskType::ImportBatch)
    , m_batchTask(task)
{
}

void ActionImportWorker::processImportFolder()
{
    PetBasicInfo unusedPetInfo;
    QList<PetAction> unusedActions;

    ActionImportResult result = ActionImportService::registerExistingAction(
        m_folderTask.petDir,
        unusedPetInfo,
        unusedActions,
        m_folderTask.currentPlaylist,
        m_folderTask.actionId,
        m_folderTask.actionFolderPath,
        m_folderTask.fps,
        m_folderTask.targetCategory,
        m_folderTask.timedIntervalSeconds,
        m_folderTask.emotionName,
        m_folderTask.timedTriggerMode,
        m_folderTask.triggerTime
    );

    emit finished(result);
}

void ActionImportWorker::processImportGif()
{
    PetBasicInfo unusedPetInfo;
    QList<PetAction> unusedActions;

    ActionImportResult result = ActionImportService::importGifAction(
        m_gifTask.petDir,
        unusedPetInfo,
        unusedActions,
        m_gifTask.currentPlaylist,
        m_gifTask.gifPath,
        m_gifTask.actionId,
        m_gifTask.fps,
        m_gifTask.targetCategory,
        m_gifTask.timedIntervalSeconds,
        m_gifTask.emotionName,
        m_gifTask.timedTriggerMode,
        m_gifTask.triggerTime
    );

    emit finished(result);
}

void ActionImportWorker::processImportBatch()
{
    ActionImportResult finalResult;
    finalResult.success = false;
    finalResult.warning = false;

    int successCount = 0;
    int skippedCount = 0;
    int failedCount = 0;

    PetBasicInfo unusedPetInfo;
    QList<PetAction> unusedActions;

    PetPlaylist workingPlaylist = m_batchTask.currentPlaylist;
    bool needTrackPlaylist = (m_batchTask.targetCategory != TargetCategory::None);

    for (const ImportActionItem &item : m_batchTask.items) {
        if (ActionLibraryIndexService::containsActionId(item.actionId)) {
            ++skippedCount;
            qWarning() << "Skipped action (already exists):" << item.actionId;
            continue;
        }

        ActionImportResult result = ActionImportService::registerExistingAction(
            m_batchTask.petDir,
            unusedPetInfo,
            unusedActions,
            workingPlaylist,
            item.actionId,
            item.sourceDir,
            item.fps,
            m_batchTask.targetCategory,
            m_batchTask.timedIntervalSeconds,
            m_batchTask.emotionName,
            m_batchTask.timedTriggerMode,
            m_batchTask.triggerTime
        );

        if (result.success) {
            ++successCount;

            if (needTrackPlaylist) {
                QString playlistPath = QDir(m_batchTask.petDir).filePath("playlist.json");
                PetPlaylist reloadedPlaylist;
                if (PetConfigManager::loadPlaylistFromJson(playlistPath, reloadedPlaylist)) {
                    workingPlaylist = reloadedPlaylist;
                } else {
                    qWarning() << "Failed to reload playlist after importing action:" << item.actionId;
                }
            }
        } else {
            ++failedCount;
            qWarning() << "Failed to import action:" << item.actionId << "-" << result.message;
        }
    }

    finalResult.success = (successCount > 0);
    finalResult.warning = (skippedCount > 0 || failedCount > 0);

    QStringList parts;
    parts << QObject::tr("成功 %1").arg(successCount);
    if (skippedCount > 0) {
        parts << QObject::tr("跳过 %1").arg(skippedCount);
    }
    if (failedCount > 0) {
        parts << QObject::tr("失败 %1").arg(failedCount);
    }

    finalResult.message = QObject::tr("动作库导入完成：%1。").arg(parts.join("，"));

    emit finished(finalResult);
}
