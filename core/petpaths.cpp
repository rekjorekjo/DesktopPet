#include "petpaths.h"

#include "core/appsettings.h"
#include "core/petconfigmanager.h"
#include "models/petaction.h"
#include "models/petplaylist.h"
#include "services/petlibraryindexservice.h"

#include <QDebug>
#include <QDir>
#include <QFile>

QString PetPaths::rootDirectory()
{
    return QDir::currentPath() + "/pets";
}

QString PetPaths::actionsDirectory()
{
    return rootDirectory() + "/actions";
}

QString PetPaths::petsDirectory()
{
    return rootDirectory() + "/pets";
}

QString PetPaths::petDirectory(const QString &petId)
{
    return petsDirectory() + "/" + petId;
}

QString PetPaths::currentPetDirectory()
{
    return petDirectory(AppSettings::currentPetId());
}

QString PetPaths::defaultPetDirectory()
{
    return petsDirectory() + "/default_pet";
}

QString PetPaths::legacyDefaultPetDirectory()
{
    return rootDirectory() + "/default_pet";
}

QString PetPaths::currentPetJsonPath()
{
    return currentPetDirectory() + "/pet.json";
}

QString PetPaths::currentPlaylistPath()
{
    return currentPetDirectory() + "/playlist.json";
}

QString PetPaths::defaultPetJsonPath()
{
    return defaultPetDirectory() + "/pet.json";
}

QString PetPaths::defaultPlaylistPath()
{
    return defaultPetDirectory() + "/playlist.json";
}

QString PetPaths::resolveActionDirectory(const PetAction &action)
{
    if (action.folderPath.isEmpty()) {
        return actionsDirectory() + "/" + action.id;
    }

    QDir dir(action.folderPath);
    if (dir.isAbsolute()) {
        return action.folderPath;
    }

    return actionsDirectory() + "/" + action.folderPath;
}

bool PetPaths::ensureDefaultStructure()
{
    QDir rootDir(rootDirectory());
    if (!rootDir.exists()) {
        if (!rootDir.mkpath(".")) {
            qWarning() << "Failed to create root directory:" << rootDirectory();
            return false;
        }
    }

    QDir actionsDir(actionsDirectory());
    if (!actionsDir.exists()) {
        if (!actionsDir.mkpath(".")) {
            qWarning() << "Failed to create actions directory:" << actionsDirectory();
            return false;
        }
    }

    QDir petsDir(petsDirectory());
    if (!petsDir.exists()) {
        if (!petsDir.mkpath(".")) {
            qWarning() << "Failed to create pets directory:" << petsDirectory();
            return false;
        }
    }

    if (!migrateFromLegacyStructure()) {
        qWarning() << "Migration from legacy structure encountered issues, but continuing...";
    }

    if (!PetLibraryIndexService::ensureLibrary()) {
        qWarning() << "Failed to ensure pet library index, but continuing...";
    }

    return true;
}

bool PetPaths::migrateFromLegacyStructure()
{
    QDir legacyDir(legacyDefaultPetDirectory());
    if (!legacyDir.exists()) {
        return true;
    }

    bool success = true;

    QString legacyPetJson = legacyDefaultPetDirectory() + "/pet.json";
    QString newPetJson = defaultPetJsonPath();
    if (QFile::exists(legacyPetJson) && !QFile::exists(newPetJson)) {
        if (QFile::copy(legacyPetJson, newPetJson)) {
            qDebug() << "Migrated pet.json from legacy to new location";
        } else {
            qWarning() << "Failed to migrate pet.json";
            success = false;
        }
    }

    QString legacyPlaylistJson = legacyDefaultPetDirectory() + "/playlist.json";
    QString newPlaylistJson = defaultPlaylistPath();
    if (QFile::exists(legacyPlaylistJson) && !QFile::exists(newPlaylistJson)) {
        if (QFile::copy(legacyPlaylistJson, newPlaylistJson)) {
            qDebug() << "Migrated playlist.json from legacy to new location";
        } else {
            qWarning() << "Failed to migrate playlist.json";
            success = false;
        }
    }

    QString legacyActionsDir = legacyDefaultPetDirectory() + "/actions";
    QDir legacyActionsDirInfo(legacyActionsDir);
    if (legacyActionsDirInfo.exists()) {
        QStringList actionFolders = legacyActionsDirInfo.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &actionFolder : actionFolders) {
            QString sourcePath = legacyActionsDir + "/" + actionFolder;
            QString destPath = actionsDirectory() + "/" + actionFolder;

            if (!QDir(destPath).exists()) {
                if (copyDirectory(sourcePath, destPath)) {
                    qDebug() << "Migrated action folder:" << actionFolder;
                } else {
                    qWarning() << "Failed to migrate action folder:" << actionFolder;
                    success = false;
                }
            } else {
                qDebug() << "Action folder already exists in new location, skipping:" << actionFolder;
            }
        }
    }

    return success;
}

bool PetPaths::copyDirectory(const QString &source, const QString &destination)
{
    QDir sourceDir(source);
    if (!sourceDir.exists()) {
        return false;
    }

    QDir destDir(destination);
    if (!destDir.mkpath(".")) {
        return false;
    }

    QStringList files = sourceDir.entryList(QDir::Files);
    for (const QString &file : files) {
        QString srcFile = source + "/" + file;
        QString destFile = destination + "/" + file;
        if (!QFile::copy(srcFile, destFile)) {
            qWarning() << "Failed to copy file:" << srcFile << "to" << destFile;
            return false;
        }
    }

    QStringList subDirs = sourceDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &subDir : subDirs) {
        QString srcSubDir = source + "/" + subDir;
        QString destSubDir = destination + "/" + subDir;
        if (!copyDirectory(srcSubDir, destSubDir)) {
            return false;
        }
    }

    return true;
}
