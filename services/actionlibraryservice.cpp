#include "actionlibraryservice.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"

#include <QDebug>
#include <QDir>
#include <QFile>
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
    const QString &actionId)
{
    ActionLibraryOperationResult result;
    result.success = false;
    result.warning = false;

    if (actionId.isEmpty()) {
        result.message = QObject::tr("动作 ID 不能为空。");
        return result;
    }
    if (actionId == "." || actionId == ".." || actionId.contains('/') || actionId.contains('\\')) {
        result.message = QObject::tr("动作 ID 无效。");
        return result;
    }

    const QString actionsBaseDir = PetPaths::actionsDirectory();
    const QString actionDirPath = QDir(actionsBaseDir).filePath(actionId);
    QDir actionDir(actionDirPath);

    bool actionDirExisted = actionDir.exists();
    bool actionDirDeleted = false;

    if (actionDirExisted) {
        const QString canonicalActionDir = actionDir.canonicalPath();
        const QString canonicalActionsDir = QDir(actionsBaseDir).canonicalPath();

        if (canonicalActionDir.isEmpty() || canonicalActionsDir.isEmpty()) {
            result.message = QObject::tr("无法验证动作目录路径。");
            return result;
        }

#ifdef Q_OS_WIN
        const QString actionDirCmp = canonicalActionDir.toLower();
        const QString actionsDirCmp = canonicalActionsDir.toLower();
#else
        const QString actionDirCmp = canonicalActionDir;
        const QString actionsDirCmp = canonicalActionsDir;
#endif

        if (actionDirCmp == actionsDirCmp) {
            result.message = QObject::tr("不允许删除动作库根目录。");
            return result;
        }

        if (!actionDirCmp.startsWith(actionsDirCmp + "/")) {
            result.message = QObject::tr("动作目录不在动作库内。");
            return result;
        }

        if (!actionDir.removeRecursively()) {
            result.message = QObject::tr("删除动作目录失败。");
            return result;
        }

        actionDirDeleted = true;
    }

    QDir petsRoot(PetPaths::petsDirectory());
    if (petsRoot.exists()) {
        const QStringList petFolders = petsRoot.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &petId : petFolders) {
            const QString playlistPath = petsRoot.filePath(petId + "/playlist.json");
            if (!QFile::exists(playlistPath)) {
                continue;
            }

            PetPlaylist playlist;
            if (!PetConfigManager::loadPlaylistFromJson(playlistPath, playlist)) {
                qWarning() << "Failed to load playlist while deleting global action:"
                           << playlistPath << "actionId:" << actionId;
                ++result.failedPlaylistCount;
                continue;
            }

            const int removedCount = playlist.removeActionReferences(actionId);
            if (removedCount <= 0) {
                continue;
            }

            if (!PetConfigManager::savePlaylistToJson(playlistPath, playlist)) {
                qWarning() << "Failed to save playlist while deleting global action:"
                           << playlistPath << "actionId:" << actionId;
                ++result.failedPlaylistCount;
                continue;
            }

            result.removedReferenceCount += removedCount;
            ++result.cleanedPetCount;
        }
    }

    result.success = true;
    result.actionDirectoryDeleted = actionDirDeleted;
    result.warning = result.failedPlaylistCount > 0;

    if (actionDirDeleted) {
        if (result.warning) {
            result.message = QObject::tr("动作已删除，已清理 %1 个引用，%2 个配置清理失败。")
                .arg(result.removedReferenceCount)
                .arg(result.failedPlaylistCount);
        } else if (result.removedReferenceCount > 0) {
            result.message = QObject::tr("动作已删除，已清理 %1 个引用。")
                .arg(result.removedReferenceCount);
        } else {
            result.message = QObject::tr("动作已删除。");
        }
    } else {
        if (result.warning) {
            result.message = QObject::tr("动作目录不存在，已清理 %1 个引用，%2 个配置清理失败。")
                .arg(result.removedReferenceCount)
                .arg(result.failedPlaylistCount);
        } else if (result.removedReferenceCount > 0) {
            result.message = QObject::tr("动作目录不存在，已清理 %1 个引用。")
                .arg(result.removedReferenceCount);
        } else {
            result.message = QObject::tr("动作目录不存在。");
        }
    }

    return result;
}

ActionLibraryOperationResult ActionLibraryService::deleteAction(
    const QString &petDir,
    QList<PetAction> actions,
    PetPlaylist playlist,
    const QString &actionId)
{
    Q_UNUSED(petDir);
    Q_UNUSED(actions);
    Q_UNUSED(playlist);

    return deleteAction(actionId);
}
