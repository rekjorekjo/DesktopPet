#include "actionlibraryservice.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"

#include <QDir>
#include <QObject>

ActionLibraryOperationResult ActionLibraryService::disableAction(
    const QString &petDir,
    QList<PetAction> actions,
    PetPlaylist playlist,
    const QString &actionId)
{
    Q_UNUSED(actions);

    ActionLibraryOperationResult result;
    result.success = false;
    result.warning = false;

    int removedCount = playlist.removeActionReferences(actionId);

    QString playlistPath = QDir(petDir).filePath("playlist.json");
    if (!PetConfigManager::savePlaylistToJson(playlistPath, playlist)) {
        result.message = QObject::tr("保存 playlist.json 失败。");
        return result;
    }

    result.success = true;
    result.message = QObject::tr("已从当前宠物移除动作 %1，清理了 %2 个分类引用。").arg(actionId).arg(removedCount);
    return result;
}

ActionLibraryOperationResult ActionLibraryService::deleteAction(
    const QString &petDir,
    QList<PetAction> actions,
    PetPlaylist playlist,
    const QString &actionId)
{
    Q_UNUSED(actions);

    ActionLibraryOperationResult result;
    result.success = false;
    result.warning = false;

    QString actionDir = PetPaths::actionsDirectory() + "/" + actionId;

    int removedCount = playlist.removeActionReferences(actionId);

    QString playlistPath = QDir(petDir).filePath("playlist.json");
    if (!PetConfigManager::savePlaylistToJson(playlistPath, playlist)) {
        result.message = QObject::tr("保存 playlist.json 失败。");
        return result;
    }

    QString actionsBaseDir = PetPaths::actionsDirectory();
    QDir dir(actionDir);

    if (!dir.exists()) {
        result.success = true;
        result.message = QObject::tr("已删除动作 %1（目录不存在），清理了 %2 个分类引用。")
            .arg(actionId).arg(removedCount);
        return result;
    }

    QString canonicalActionDir = QDir(actionDir).canonicalPath();
    QString canonicalActionsDir = QDir(actionsBaseDir).canonicalPath();

    if (canonicalActionDir.isEmpty() || canonicalActionsDir.isEmpty()) {
        result.success = true;
        result.warning = true;
        result.message = QObject::tr("已删除动作 %1，清理了 %2 个分类引用，但无法验证动作目录路径。")
            .arg(actionId).arg(removedCount);
        return result;
    }

#ifdef Q_OS_WIN
    QString actionDirCmp = canonicalActionDir.toLower();
    QString actionsDirCmp = canonicalActionsDir.toLower();
#else
    QString actionDirCmp = canonicalActionDir;
    QString actionsDirCmp = canonicalActionsDir;
#endif

    if (actionDirCmp == actionsDirCmp) {
        result.success = true;
        result.warning = true;
        result.message = QObject::tr("已删除动作 %1，清理了 %2 个分类引用，但不允许删除动作库根目录。")
            .arg(actionId).arg(removedCount);
        return result;
    }

    if (!actionDirCmp.startsWith(actionsDirCmp + "/")) {
        result.success = true;
        result.warning = true;
        result.message = QObject::tr("已删除动作 %1，清理了 %2 个分类引用，但动作目录不在动作库内。")
            .arg(actionId).arg(removedCount);
        return result;
    }

    if (!dir.removeRecursively()) {
        result.success = true;
        result.warning = true;
        result.message = QObject::tr("已删除动作 %1，清理了 %2 个分类引用，但删除动作目录失败。")
            .arg(actionId).arg(removedCount);
        return result;
    }

    result.success = true;
    result.message = QObject::tr("已删除动作 %1，清理了 %2 个分类引用。").arg(actionId).arg(removedCount);
    return result;
}
