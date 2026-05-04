#ifndef ACTIONIMPORTSERVICE_H
#define ACTIONIMPORTSERVICE_H

#include <QString>
#include <QList>

struct PetAction;
struct PetBasicInfo;
class PetPlaylist;

enum class TargetCategory
{
    None,
    Idle,
    Random,
    Timed,
    Emotion
};

struct ActionImportResult
{
    bool success = false;
    bool warning = false;
    QString message;
};

class ActionImportService
{
public:
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
        const QString &emotionName
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
        const QString &emotionName
    );

private:
    static ActionImportResult addToCategory(
        const QString &petDir,
        PetPlaylist &playlist,
        const QString &actionId,
        TargetCategory targetCategory,
        int timedIntervalSeconds,
        const QString &emotionName
    );
};

#endif // ACTIONIMPORTSERVICE_H
