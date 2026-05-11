#ifndef ACTIONLIBRARYSERVICE_H
#define ACTIONLIBRARYSERVICE_H

#include <QList>
#include <QString>

#include "models/petaction.h"
#include "models/petplaylist.h"

struct ActionLibraryOperationResult
{
    bool success = false;
    bool warning = false;
    QString message;
    int removedReferenceCount = 0;
    int cleanedPetCount = 0;
    int failedPlaylistCount = 0;
    bool actionDirectoryDeleted = false;
    int replacedReferenceCount = 0;
};

class ActionLibraryService
{
public:
    static ActionLibraryOperationResult disableAction(
        const QString &petDir,
        QList<PetAction> actions,
        PetPlaylist playlist,
        const QString &actionId
    );

    // 左侧"移除动作"的业务逻辑
    // 只从 actionlibrary.json 移除 entry
    // 不删除 actions/<actionId>/ 目录
    // 同时清理 petlibrary 中所有宠物 playlist 引用
    // 这样 actionId 后续可以重新导入 / 新建
    static ActionLibraryOperationResult removeAction(
        const QString &actionId
    );

    // 左侧"删除动作"的业务逻辑
    // 删除 actionlibrary entry
    // 删除 actions/<dir>/ 目录
    // 清理 petlibrary 中所有宠物 playlist 引用
    static ActionLibraryOperationResult deleteAction(
        const QString &actionId
    );

    static ActionLibraryOperationResult deleteAction(
        const QString &petDir,
        QList<PetAction> actions,
        PetPlaylist playlist,
        const QString &actionId
    );

    static ActionLibraryOperationResult renameActionId(
        const QString &oldActionId,
        const QString &newActionId
    );
};

#endif // ACTIONLIBRARYSERVICE_H
