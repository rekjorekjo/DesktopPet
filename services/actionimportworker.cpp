#include "actionimportworker.h"

#include "core/petaction.h"
#include "core/petconfigmanager.h"
#include "services/actionimportservice.h"

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
