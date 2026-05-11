#include "actionlibraryindexservice.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

QString ActionLibraryIndexService::libraryFilePath()
{
    return PetPaths::actionsDirectory() + "/actionlibrary.json";
}

// 确保动作库目录和索引文件存在
//
// 如果 pets/actions 目录不存在则创建，
// 如果 actionlibrary.json 不存在则创建空索引文件。
// 这是动作库正常工作的前提条件。
bool ActionLibraryIndexService::ensureLibrary()
{
    QString actionsDirPath = PetPaths::actionsDirectory();
    QDir actionsDir(actionsDirPath);

    if (!actionsDir.exists()) {
        if (!actionsDir.mkpath(".")) {
            qWarning() << "Failed to create actions directory:" << actionsDirPath;
            return false;
        }
    }

    QString libraryPath = libraryFilePath();

    if (QFile::exists(libraryPath)) {
        return true;
    }

    QList<ActionLibraryEntry> entries = scanExistingActions();

    return saveEntries(entries);
}

QList<ActionLibraryEntry> ActionLibraryIndexService::loadEntries()
{
    ensureLibrary();
    return loadEntriesFromFile(libraryFilePath());
}

QList<ActionLibraryEntry> ActionLibraryIndexService::loadEntriesFromFile(const QString &filePath)
{
    QList<ActionLibraryEntry> entries;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open actionlibrary.json for reading:" << filePath;
        return entries;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse actionlibrary.json:" << parseError.errorString();
        return entries;
    }

    QJsonObject root = doc.object();
    QJsonArray actionsArray = root["actions"].toArray();

    for (const QJsonValue &value : actionsArray) {
        QJsonObject actionObj = value.toObject();
        ActionLibraryEntry entry;
        entry.id = actionObj["id"].toString();
        entry.name = actionObj["name"].toString();
        entry.dir = actionObj["dir"].toString();

        if (!entry.id.isEmpty()) {
            entries.append(entry);
        }
    }

    return entries;
}

// 保存动作库索引到文件
//
// 使用 QSaveFile 而非 QFile，确保写入原子性：
// - 写入过程中断不会损坏原文件
// - 只有写入成功才会替换原文件
bool ActionLibraryIndexService::saveEntries(const QList<ActionLibraryEntry> &entries)
{
    QString libraryPath = libraryFilePath();

    QDir dir(QFileInfo(libraryPath).absolutePath());
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create actionlibrary parent directory:" << dir.absolutePath();
            return false;
        }
    }

    QJsonArray actionsArray;
    for (const ActionLibraryEntry &entry : entries) {
        QJsonObject actionObj;
        actionObj["id"] = entry.id;
        actionObj["name"] = entry.name;
        actionObj["dir"] = entry.dir;
        actionsArray.append(actionObj);
    }

    QJsonObject root;
    root["version"] = 1;
    root["actions"] = actionsArray;

    QJsonDocument doc(root);

    QSaveFile file(libraryPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open actionlibrary.json for writing:" << libraryPath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    return file.commit();
}

bool ActionLibraryIndexService::containsActionId(const QString &actionId)
{
    if (actionId.isEmpty()) {
        return false;
    }

    QList<ActionLibraryEntry> entries = loadEntries();
    for (const ActionLibraryEntry &entry : entries) {
        if (entry.id == actionId) {
            return true;
        }
    }

    return false;
}

std::optional<ActionLibraryEntry> ActionLibraryIndexService::findAction(const QString &actionId)
{
    if (actionId.isEmpty()) {
        return std::nullopt;
    }

    QList<ActionLibraryEntry> entries = loadEntries();
    for (const ActionLibraryEntry &entry : entries) {
        if (entry.id == actionId) {
            return entry;
        }
    }

    return std::nullopt;
}

bool ActionLibraryIndexService::addOrUpdateAction(const ActionLibraryEntry &entry)
{
    if (entry.id.isEmpty()) {
        return false;
    }

    QList<ActionLibraryEntry> entries = loadEntries();

    bool found = false;
    for (int i = 0; i < entries.size(); ++i) {
        if (entries[i].id == entry.id) {
            entries[i] = entry;
            found = true;
            break;
        }
    }

    if (!found) {
        entries.append(entry);
    }

    return saveEntries(entries);
}

bool ActionLibraryIndexService::removeActionEntry(const QString &actionId)
{
    if (actionId.isEmpty()) {
        return false;
    }

    QList<ActionLibraryEntry> entries = loadEntries();

    for (int i = 0; i < entries.size(); ++i) {
        if (entries[i].id == actionId) {
            entries.removeAt(i);
            break;
        }
    }

    return saveEntries(entries);
}

// 删除动作条目及其目录
//
// 优先使用 entry.dir 而非 actionId 作为目录名，
// 因为 entry.dir 记录的是实际存储路径，可能与 actionId 不同。
// 删除前会验证目录在 actions 根目录内，防止误删外部路径。
bool ActionLibraryIndexService::deleteActionEntryAndDirectory(const QString &actionId)
{
    if (actionId.isEmpty()) {
        return false;
    }

    auto entryOpt = findAction(actionId);
    QString dirName = entryOpt.has_value() && !entryOpt->dir.isEmpty()
        ? entryOpt->dir
        : actionId;

    QString actionsBaseDir = PetPaths::actionsDirectory();
    QString actionDir = QDir(actionsBaseDir).filePath(dirName);
    QDir dir(actionDir);

    if (dir.exists()) {
        QString canonicalActionsDir = QDir(actionsBaseDir).canonicalPath();
        QString canonicalActionDir = dir.canonicalPath();

        if (canonicalActionsDir.isEmpty() || canonicalActionDir.isEmpty()) {
            qWarning() << "Failed to verify action directory path for deletion:" << actionDir;
            return false;
        }

#ifdef Q_OS_WIN
        QString actionsDirCmp = canonicalActionsDir.toLower();
        QString actionDirCmp = canonicalActionDir.toLower();
#else
        QString actionsDirCmp = canonicalActionsDir;
        QString actionDirCmp = canonicalActionDir;
#endif

        if (actionDirCmp == actionsDirCmp) {
            qWarning() << "Cannot delete actions root directory";
            return false;
        }

        if (!actionDirCmp.startsWith(actionsDirCmp + "/")) {
            qWarning() << "Action directory is not inside actions root:" << actionDir;
            return false;
        }

        if (!dir.removeRecursively()) {
            qWarning() << "Failed to remove action directory:" << actionDir;
            return false;
        }
    }

    removeActionEntry(actionId);

    return true;
}

QList<ActionLibraryEntry> ActionLibraryIndexService::scanExistingActions()
{
    QList<ActionLibraryEntry> entries;

    QString actionsDirPath = PetPaths::actionsDirectory();
    QDir actionsDir(actionsDirPath);

    if (!actionsDir.exists()) {
        return entries;
    }

    QStringList actionFolders = actionsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &actionId : actionFolders) {
        QString actionDir = PetPaths::actionsDirectory() + "/" + actionId;

        QStringList frameFiles = PetConfigManager::scanFrameFiles(actionDir);
        if (frameFiles.isEmpty()) {
            continue;
        }

        ActionLibraryEntry entry;
        entry.id = actionId;
        entry.dir = actionId;

        PetAction actionInfo = PetConfigManager::loadGlobalActionFromDirectory(actionId, actionDir);

        if (actionInfo.isValid()) {
            entry.name = actionInfo.name.isEmpty() ? actionId : actionInfo.name;
        } else {
            entry.name = actionId;
        }

        entries.append(entry);
    }

    return entries;
}
