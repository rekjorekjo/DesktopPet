#ifndef ACTIONIMPORTSERVICE_H
#define ACTIONIMPORTSERVICE_H

#include <QMetaType>
#include <QString>
#include <QList>

#include "models/actioncategory.h"
#include "models/petplaylist.h"

struct PetAction;
struct PetBasicInfo;
class PetPlaylist;

// 动作导入结果
struct ActionImportResult
{
    bool success = false;
    bool warning = false;
    QString message;
};

Q_DECLARE_METATYPE(ActionImportResult)

// 单动作/动作库导入的业务逻辑服务
//
// 职责：
// - 将全局动作注册到当前宠物的 playlist
// - 导入 GIF 文件作为新动作
// - 导入本地动作文件夹
//
// 导入流程：
// 1. 验证源数据有效性
// 2. 检查 ID 冲突
// 3. 复制资源文件到目标目录
// 4. 更新 actionlibrary.json
// 5. 更新 playlist.json
class ActionImportService
{
public:
    // 将全局动作库中已有的动作注册到当前宠物
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

    // 导入本地动作文件夹到当前宠物
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

    // 导入 GIF 文件作为新动作
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
