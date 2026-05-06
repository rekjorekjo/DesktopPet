#ifndef ACTIONIMPORTWORKER_H
#define ACTIONIMPORTWORKER_H

#include <QObject>

#include "models/actioncategory.h"
#include "models/petplaylist.h"
#include "services/actionimportservice.h"

struct PetAction;
class PetPlaylist;

class ActionImportWorker : public QObject
{
    Q_OBJECT

public:
    enum class TaskType
    {
        ImportFolder,
        ImportGif
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

    explicit ActionImportWorker(const ImportFolderTask &task, QObject *parent = nullptr);
    explicit ActionImportWorker(const ImportGifTask &task, QObject *parent = nullptr);

public slots:
    void processImportFolder();
    void processImportGif();

signals:
    void finished(const ActionImportResult &result);

private:
    TaskType m_taskType;
    ImportFolderTask m_folderTask;
    ImportGifTask m_gifTask;
};

#endif // ACTIONIMPORTWORKER_H
