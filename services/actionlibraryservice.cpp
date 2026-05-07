#include "actionlibraryservice.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QRegularExpression>

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

ActionLibraryOperationResult ActionLibraryService::renameActionId(
    const QString &oldActionId,
    const QString &newActionId)
{
    ActionLibraryOperationResult result;
    result.success = false;
    result.warning = false;

    QString trimmedOld = oldActionId.trimmed();
    QString trimmedNew = newActionId.trimmed();

    if (trimmedOld.isEmpty()) {
        result.message = QObject::tr("原动作 ID 不能为空。");
        return result;
    }

    if (trimmedNew.isEmpty()) {
        result.message = QObject::tr("新动作 ID 不能为空。");
        return result;
    }

    if (trimmedOld == "." || trimmedOld == ".." || trimmedOld.contains('/') || trimmedOld.contains('\\')) {
        result.message = QObject::tr("原动作 ID 无效。");
        return result;
    }

    if (trimmedNew == "." || trimmedNew == ".." || trimmedNew.contains('/') || trimmedNew.contains('\\')) {
        result.message = QObject::tr("新动作 ID 不能包含 / 或 \\。");
        return result;
    }

    static const QRegularExpression validIdPattern("^[a-zA-Z0-9_-]+$");
    if (!validIdPattern.match(trimmedNew).hasMatch()) {
        result.message = QObject::tr("新动作 ID 只能包含字母、数字、下划线和短横线。");
        return result;
    }

    if (trimmedOld == trimmedNew) {
        result.message = QObject::tr("新动作 ID 与原动作 ID 相同，无需修改。");
        return result;
    }

    const QString actionsBaseDir = PetPaths::actionsDirectory();
    const QString oldDirPath = QDir(actionsBaseDir).filePath(trimmedOld);
    const QString newDirPath = QDir(actionsBaseDir).filePath(trimmedNew);

    QDir oldDir(oldDirPath);
    if (!oldDir.exists()) {
        result.message = QObject::tr("原动作目录不存在。");
        return result;
    }

    QDir newDir(newDirPath);
    if (newDir.exists()) {
        result.message = QObject::tr("目标动作 ID 已存在，请选择其他名称。");
        return result;
    }

    const QString canonicalOldDir = oldDir.canonicalPath();
    const QString canonicalActionsDir = QDir(actionsBaseDir).canonicalPath();

    if (canonicalOldDir.isEmpty() || canonicalActionsDir.isEmpty()) {
        result.message = QObject::tr("无法验证动作目录路径。");
        return result;
    }

#ifdef Q_OS_WIN
    const QString oldDirCmp = canonicalOldDir.toLower();
    const QString actionsDirCmp = canonicalActionsDir.toLower();
#else
    const QString oldDirCmp = canonicalOldDir;
    const QString actionsDirCmp = canonicalActionsDir;
#endif

    if (oldDirCmp == actionsDirCmp) {
        result.message = QObject::tr("不允许重命名动作库根目录。");
        return result;
    }

    if (!oldDirCmp.startsWith(actionsDirCmp + "/")) {
        result.message = QObject::tr("动作目录不在动作库内。");
        return result;
    }

    if (!oldDir.rename(oldDirPath, newDirPath)) {
        result.message = QObject::tr("重命名动作目录失败。");
        return result;
    }

    QString actionJsonPath = QDir(newDirPath).filePath("action.json");
    if (QFile::exists(actionJsonPath)) {
        QFile jsonFile(actionJsonPath);
        if (jsonFile.open(QIODevice::ReadWrite)) {
            QByteArray jsonData = jsonFile.readAll();
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

            if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
                QJsonObject obj = doc.object();
                obj["id"] = trimmedNew;

                QString currentName = obj.value("name").toString().trimmed();
                if (currentName.isEmpty() || currentName == trimmedOld) {
                    obj["name"] = trimmedNew;
                }

                jsonFile.seek(0);
                jsonFile.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
                jsonFile.resize(jsonFile.pos());
            } else {
                qWarning() << "Failed to parse action.json during rename:" << actionJsonPath;
                result.warning = true;
            }
            jsonFile.close();
        } else {
            qWarning() << "Failed to open action.json for writing during rename:" << actionJsonPath;
            result.warning = true;
        }
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
                qWarning() << "Failed to load playlist while renaming global action:"
                           << playlistPath << "oldActionId:" << trimmedOld;
                ++result.failedPlaylistCount;
                continue;
            }

            const int replacedCount = playlist.replaceActionReferences(trimmedOld, trimmedNew);
            if (replacedCount <= 0) {
                continue;
            }

            if (!PetConfigManager::savePlaylistToJson(playlistPath, playlist)) {
                qWarning() << "Failed to save playlist while renaming global action:"
                           << playlistPath << "oldActionId:" << trimmedOld;
                ++result.failedPlaylistCount;
                continue;
            }

            result.replacedReferenceCount += replacedCount;
            ++result.cleanedPetCount;
        }
    }

    result.success = true;
    result.warning = result.failedPlaylistCount > 0;

    if (result.warning) {
        result.message = QObject::tr("动作 ID 已重命名为 %1，已更新 %2 个引用，%3 个配置更新失败。")
            .arg(trimmedNew)
            .arg(result.replacedReferenceCount)
            .arg(result.failedPlaylistCount);
    } else if (result.replacedReferenceCount > 0) {
        result.message = QObject::tr("动作 ID 已重命名为 %1，已更新 %2 个引用。")
            .arg(trimmedNew)
            .arg(result.replacedReferenceCount);
    } else {
        result.message = QObject::tr("动作 ID 已重命名为 %1。").arg(trimmedNew);
    }

    return result;
}
