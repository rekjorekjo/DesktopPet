#ifndef ACTIONIMPORTSERVICE_H
#define ACTIONIMPORTSERVICE_H

#include <QMetaType>
#include <QString>
#include <QList>

#include "core/actioncategory.h"
#include "core/petplaylist.h"

struct PetAction;
struct PetBasicInfo;
class PetPlaylist;

struct ActionImportResult
{
    bool success = false;
    bool warning = false;
    QString message;
};

Q_DECLARE_METATYPE(ActionImportResult)

class ActionImportService
{
public:
    static ActionImportResult registerGlobalActionToPet(
        const QString &petDir,
        const PetBasicInfo &petInfo,
        QList<PetAction> currentActions,
        PetPlaylist currentPlaylist,
        const QString &actionId,
        int fps,
        TargetCategory targetCategory,
        int timedIntervalSeconds,
        const QString &emotionName,
        TimedTriggerMode timedTriggerMode = TimedTriggerMode::Interval,
        const QString &triggerTime = "00:00"
    );

    static ActionImportResult registerExistingAction(
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
        TimedTriggerMode timedTriggerMode = TimedTriggerMode::Interval,
        const QString &triggerTime = "00:00"
    );

    static ActionImportResult importGifAction(
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
        TimedTriggerMode timedTriggerMode = TimedTriggerMode::Interval,
        const QString &triggerTime = "00:00"
    );

private:
    static ActionImportResult addToCategory(
        const QString &petDir,
        PetPlaylist &playlist,
        const QString &actionId,
        TargetCategory targetCategory,
        int timedIntervalSeconds,
        const QString &emotionName,
        TimedTriggerMode timedTriggerMode = TimedTriggerMode::Interval,
        const QString &triggerTime = "00:00"
    );
};

#endif // ACTIONIMPORTSERVICE_H
