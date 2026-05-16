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

    QList<PetLibraryEntry> entries = scanExistingPets();

    if (entries.isEmpty()) {
        QString defaultPetDir = PetPaths::petDirectory("default_pet");
        QDir dir;
        if (!dir.mkpath(defaultPetDir)) {
            qWarning() << "Failed to create default pet directory:" << defaultPetDir;
            return false;
        }

        QString petJsonPath = defaultPetDir + "/pet.json";
        QString playlistPath = defaultPetDir + "/playlist.json";

        PetBasicInfo info;
        info.id = "default_pet";
        info.name = "Default Pet";
        info.canvasSize = QSize(400, 400);
        info.displaySize = QSize(200, 200);

        if (!PetConfigManager::savePetInfoJson(petJsonPath, info)) {
            qWarning() << "Failed to create default pet.json";
            return false;
        }

        PetPlaylist playlist;
        if (!PetConfigManager::savePlaylistToJson(playlistPath, playlist)) {
            qWarning() << "Failed to create default playlist.json";
            return false;
        }

        PetLibraryEntry entry;
        entry.id = "default_pet";
        entry.name = "Default Pet";
        entry.dir = "pets/default_pet";
        entry.enabled = true;
        entries.append(entry);
    }

    return saveEntries(entries);
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

    QString currentPetId = AppSettings::currentPetId();

    QList<PetLibraryEntry> entries = loadEntries();

    if (!currentPetId.isEmpty()) {
        for (const PetLibraryEntry &entry : entries) {
            if (entry.id == currentPetId) {
                return currentPetId;
            }
        }
    }

    if (!entries.isEmpty()) {
        QString firstId = entries.first().id;
        AppSettings::setCurrentPetId(firstId);
        return firstId;
    }

    AppSettings::setCurrentPetId("");
    return QString();
}

QList<PetLibraryEntry> PetLibraryIndexService::scanExistingPets()
{
    QList<PetLibraryEntry> entries;

    QString petsDirPath = PetPaths::petsDirectory();
    QDir petsDir(petsDirPath);

    if (!petsDir.exists()) {
        return entries;
    }

    QStringList petFolders = petsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &petId : petFolders) {
        QString petDir = PetPaths::petDirectory(petId);
        QString petJsonPath = petDir + "/pet.json";

        PetLibraryEntry entry;
        entry.id = petId;
        entry.dir = "pets/" + petId;
        entry.enabled = true;

        PetBasicInfo info;
        if (PetConfigManager::loadPetInfoJson(petJsonPath, info)) {
            entry.name = info.name.isEmpty() ? petId : info.name;
        } else {
            entry.name = petId;
        }

        entries.append(entry);
    }

    return entries;
}
