#ifndef ACTIONIMPORTWORKER_H
#define ACTIONIMPORTWORKER_H

#include <QObject>

#include "dialogs/importactiondialog.h"
#include "models/actioncategory.h"
#include "models/petplaylist.h"
#include "services/actionimportservice.h"

struct PetAction;
class PetPlaylist;

struct BatchImportResult
{
    int successCount = 0;
    int skippedCount = 0;
    int failedCount = 0;
    QString message;
};

class ActionImportWorker : public QObject
{
    Q_OBJECT

public:
    enum class TaskType
    {
        ImportFolder,
        ImportGif,
        ImportBatch
    };

    struct ImportFolderTask
    {
        QString petDir;
        PetPlaylist currentPlaylist;

        QString actionFolderPath;
        QString actionId;
        int fps = 12;

        TargetCategory targetCategory = TargetCategory::None;
        int timedIntervalSeconds = 300;
        QString emotionName = "happy";
        TimedTriggerMode timedTriggerMode = TimedTriggerMode::Interval;
        QString triggerTime = "00:00";
    };

    struct ImportGifTask
    {
        QString petDir;
        PetPlaylist currentPlaylist;

        QString gifPath;
        QString actionId;
        int fps = 12;

        TargetCategory targetCategory = TargetCategory::None;
        int timedIntervalSeconds = 300;
        QString emotionName = "happy";
        TimedTriggerMode timedTriggerMode = TimedTriggerMode::Interval;
        QString triggerTime = "00:00";
    };

    struct ImportBatchTask
    {
        QString petDir;
        PetPlaylist currentPlaylist;

        QList<ImportActionItem> items;
        ImportActionMode mode = ImportActionMode::Invalid;

        TargetCategory targetCategory = TargetCategory::None;
        int timedIntervalSeconds = 300;
        QString emotionName = "happy";
        TimedTriggerMode timedTriggerMode = TimedTriggerMode::Interval;
        QString triggerTime = "00:00";
    };

    explicit ActionImportWorker(const ImportFolderTask &task, QObject *parent = nullptr);
    explicit ActionImportWorker(const ImportGifTask &task, QObject *parent = nullptr);
    explicit ActionImportWorker(const ImportBatchTask &task, QObject *parent = nullptr);

public slots:
    void processImportFolder();
    void processImportGif();
    void processImportBatch();

signals:
    void finished(const ActionImportResult &result);

private:
    TaskType m_taskType;
    ImportFolderTask m_folderTask;
    ImportGifTask m_gifTask;
    ImportBatchTask m_batchTask;
};

#endif // ACTIONIMPORTWORKER_H
