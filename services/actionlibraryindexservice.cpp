#include "actionlibraryindexservice.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

QString ActionLibraryIndexService::libraryFilePath()
{
    return PetPaths::actionsDirectory() + "/actionlibrary.json";
}

bool ActionLibraryIndexService::ensureLibrary()
{
    QString libraryPath = libraryFilePath();

    if (QFile::exists(libraryPath)) {
        return true;
    }

    QList<ActionLibraryEntry> entries = scanExistingActions();

    return saveEntries(entries);
}

QList<ActionLibraryEntry> ActionLibraryIndexService::loadEntries()
{
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

bool ActionLibraryIndexService::saveEntries(const QList<ActionLibraryEntry> &entries)
{
    QString libraryPath = libraryFilePath();

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
