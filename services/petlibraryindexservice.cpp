#include "petlibraryindexservice.h"

#include "core/appsettings.h"
#include "core/petconfigmanager.h"
#include "core/petpaths.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QSet>

// petlibrary.json 是宠物 ID 是否存在的唯一依据。
// 目录存在但 library 无 entry 时，ID 可复用。
// 移除宠物只删 entry，不删目录。
QString PetLibraryIndexService::libraryFilePath()
{
    return PetPaths::rootDirectory() + "/petlibrary.json";
}

bool PetLibraryIndexService::ensureLibrary()
{
    QString libraryPath = libraryFilePath();

    if (QFile::exists(libraryPath)) {
        return true;
    }

    if (recoverLibraryFromDiskIfEmpty()) {
        return true;
    }

    saveEntries({});
    AppSettings::setCurrentPetId("");
    return true;
}

QList<PetLibraryEntry> PetLibraryIndexService::loadEntries()
{
    QList<PetLibraryEntry> entries;

    QString libraryPath = libraryFilePath();
    QFile file(libraryPath);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open petlibrary.json for reading:" << libraryPath;
        return entries;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse petlibrary.json:" << parseError.errorString();
        return entries;
    }

    QJsonObject root = doc.object();
    QJsonArray petsArray = root["pets"].toArray();

    for (const QJsonValue &value : petsArray) {
        QJsonObject petObj = value.toObject();
        PetLibraryEntry entry;
        entry.id = petObj["id"].toString();
        entry.name = petObj["name"].toString();
        entry.dir = petObj["dir"].toString();
        entry.enabled = petObj["enabled"].toBool(true);

        if (!entry.id.isEmpty()) {
            entries.append(entry);
        }
    }

    return entries;
}

bool PetLibraryIndexService::saveEntries(const QList<PetLibraryEntry> &entries)
{
    QString libraryPath = libraryFilePath();

    QJsonArray petsArray;
    for (const PetLibraryEntry &entry : entries) {
        QJsonObject petObj;
        petObj["id"] = entry.id;
        petObj["name"] = entry.name;
        petObj["dir"] = entry.dir;
        petObj["enabled"] = entry.enabled;
        petsArray.append(petObj);
    }

    QJsonObject root;
    root["version"] = 1;
    root["pets"] = petsArray;

    QJsonDocument doc(root);

    QSaveFile file(libraryPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open petlibrary.json for writing:" << libraryPath;
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    return file.commit();
}

bool PetLibraryIndexService::containsPetId(const QString &petId)
{
    if (petId.isEmpty()) {
        return false;
    }

    QList<PetLibraryEntry> entries = loadEntries();
    for (const PetLibraryEntry &entry : entries) {
        if (entry.id == petId) {
            return true;
        }
    }

    return false;
}

std::optional<PetLibraryEntry> PetLibraryIndexService::findPet(const QString &petId)
{
    if (petId.isEmpty()) {
        return std::nullopt;
    }

    QList<PetLibraryEntry> entries = loadEntries();
    for (const PetLibraryEntry &entry : entries) {
        if (entry.id == petId) {
            return entry;
        }
    }

    return std::nullopt;
}

bool PetLibraryIndexService::addOrUpdatePet(const PetLibraryEntry &entry)
{
    if (entry.id.isEmpty()) {
        return false;
    }

    QList<PetLibraryEntry> entries = loadEntries();

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

// 从 petlibrary.json 中移除宠物
//
// 移除宠物只删 library entry，不删目录。
// 要删除目录需调用 deletePetEntryAndDirectory()。
bool PetLibraryIndexService::removePetEntry(const QString &petId)
{
    if (petId.isEmpty()) {
        return false;
    }

    QList<PetLibraryEntry> entries = loadEntries();

    for (int i = 0; i < entries.size(); ++i) {
        if (entries[i].id == petId) {
            entries.removeAt(i);
            break;
        }
    }

    return saveEntries(entries);
}

bool PetLibraryIndexService::deletePetEntryAndDirectory(const QString &petId)
{
    if (petId.isEmpty()) {
        return false;
    }

    removePetEntry(petId);

    QString petDir = PetPaths::petDirectory(petId);
    QDir dir(petDir);

    if (dir.exists()) {
        QString canonicalPetDir = dir.canonicalPath();
        QString petsDir = QDir(PetPaths::petsDirectory()).canonicalPath();

        if (!canonicalPetDir.isEmpty() && !petsDir.isEmpty()) {
            if (canonicalPetDir.startsWith(petsDir + "/")) {
                if (!dir.removeRecursively()) {
                    qWarning() << "Failed to remove pet directory:" << petDir;
                    return false;
                }
            }
        }
    }

    return true;
}

QString PetLibraryIndexService::findFirstEnabledPetId(const QString &excludePetId)
{
    QList<PetLibraryEntry> entries = loadEntries();

    for (const PetLibraryEntry &entry : entries) {
        if (entry.id == excludePetId) {
            continue;
        }

        return entry.id;
    }

    return QString();
}

QString PetLibraryIndexService::ensureValidCurrentPetId()
{
    ensureLibrary();
    recoverLibraryFromDiskIfEmpty();

    QString currentPetId = AppSettings::currentPetId();
    QList<PetLibraryEntry> entries = loadEntries();

    if (!currentPetId.isEmpty()) {
        for (const PetLibraryEntry &entry : entries) {
            if (entry.id == currentPetId) {
                QString petDir = PetPaths::petDirectory(entry.id);
                if (isPetDirectoryValid(petDir)) {
                    return currentPetId;
                }
            }
        }
    }

    for (const PetLibraryEntry &entry : entries) {
        QString petDir = PetPaths::petDirectory(entry.id);
        if (isPetDirectoryValid(petDir)) {
            AppSettings::setCurrentPetId(entry.id);
            return entry.id;
        }
    }

    AppSettings::setCurrentPetId("");
    return QString();
}

bool PetLibraryIndexService::isPetDirectoryValid(const QString &petDir)
{
    QString petJsonPath = petDir + "/pet.json";
    QString playlistPath = petDir + "/playlist.json";

    if (!QFile::exists(petJsonPath) || !QFile::exists(playlistPath)) {
        return false;
    }

    PetBasicInfo info;
    return PetConfigManager::loadPetInfoJson(petJsonPath, info) && !info.id.isEmpty();
}

bool PetLibraryIndexService::recoverLibraryFromDiskIfEmpty()
{
    QList<PetLibraryEntry> entries = loadEntries();

    bool needsRecovery = entries.isEmpty();
    if (!needsRecovery) {
        bool hasValid = false;
        for (const PetLibraryEntry &entry : entries) {
            QString petDir = PetPaths::petDirectory(entry.id);
            if (isPetDirectoryValid(petDir)) {
                hasValid = true;
                break;
            }
        }
        needsRecovery = !hasValid;
    }

    if (!needsRecovery) {
        return false;
    }

    QString petsDirPath = PetPaths::petsDirectory();
    QDir petsDir(petsDirPath);
    if (!petsDir.exists()) {
        return false;
    }

    QSet<QString> seenIds;
    QList<PetLibraryEntry> recovered;

    QStringList petFolders = petsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &folderName : petFolders) {
        QString petDir = PetPaths::petDirectory(folderName);
        if (!isPetDirectoryValid(petDir)) {
            continue;
        }

        QString petJsonPath = petDir + "/pet.json";
        PetBasicInfo info;
        PetConfigManager::loadPetInfoJson(petJsonPath, info);

        if (info.id != folderName) {
            qWarning() << "Skipping pet directory for recovery: folderName" << folderName
                        << "!= info.id" << info.id;
            continue;
        }

        if (seenIds.contains(info.id)) {
            continue;
        }
        seenIds.insert(info.id);

        PetLibraryEntry entry;
        entry.id = info.id;
        entry.name = info.name.isEmpty() ? info.id : info.name;
        entry.dir = "pets/" + folderName;
        entry.enabled = true;
        recovered.append(entry);
    }

    if (recovered.isEmpty()) {
        return false;
    }

    if (!saveEntries(recovered)) {
        return false;
    }

    QString currentPetId = AppSettings::currentPetId();
    bool currentValid = false;
    if (!currentPetId.isEmpty()) {
        for (const PetLibraryEntry &entry : recovered) {
            if (entry.id == currentPetId) {
                currentValid = true;
                break;
            }
        }
    }
    if (!currentValid) {
        AppSettings::setCurrentPetId(recovered.first().id);
    }

    return true;
}
