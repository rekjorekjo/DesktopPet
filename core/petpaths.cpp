#include "petpaths.h"

#include "core/apppaths.h"
#include "core/appsettings.h"
#include "models/petaction.h"
#include "services/actionlibraryindexservice.h"
#include "services/petlibraryindexservice.h"

#include <QDebug>
#include <QDir>

static QString s_dataRootOverride;

void PetPaths::setDataRootOverrideForTesting(const QString &path)
{
    s_dataRootOverride = path;
}

void PetPaths::clearDataRootOverrideForTesting()
{
    s_dataRootOverride.clear();
}

QString PetPaths::rootDirectory()
{
    if (!s_dataRootOverride.isEmpty())
        return s_dataRootOverride;
    return AppPaths::dataRootDir();
}

QString PetPaths::actionsDirectory()
{
    if (!s_dataRootOverride.isEmpty())
        return rootDirectory() + "/actions";
    return AppPaths::actionsDir();
}

QString PetPaths::petsDirectory()
{
    if (!s_dataRootOverride.isEmpty())
        return rootDirectory() + "/pets";
    return AppPaths::petsDir();
}

QString PetPaths::petDirectory(const QString &petId)
{
    return petsDirectory() + "/" + petId;
}

QString PetPaths::currentPetDirectory()
{
    return petDirectory(AppSettings::currentPetId());
}

QString PetPaths::currentPetJsonPath()
{
    return currentPetDirectory() + "/pet.json";
}

QString PetPaths::currentPlaylistPath()
{
    return currentPetDirectory() + "/playlist.json";
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

QString PetPaths::configDir()
{
    if (!s_dataRootOverride.isEmpty())
        return rootDirectory() + "/config";
    return AppPaths::configDir();
}

QString PetPaths::apiProfilesFilePath()
{
    return configDir() + "/api_profiles.json";
}

QString PetPaths::chatSettingsFilePath()
{
    return configDir() + "/chat_settings.json";
}

QString PetPaths::webSearchSettingsFilePath()
{
    return configDir() + "/websearch_settings.json";
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

    QDir cfgDir(configDir());
    if (!cfgDir.exists()) {
        if (!cfgDir.mkpath(".")) {
            qWarning() << "Failed to create config directory:" << configDir();
            return false;
        }
    }

    if (!PetLibraryIndexService::ensureLibrary()) {
        qWarning() << "Failed to ensure pet library index, but continuing...";
    }

    PetLibraryIndexService::ensureValidCurrentPetId();

    if (!ActionLibraryIndexService::ensureLibrary()) {
        qWarning() << "Failed to ensure action library index, but continuing...";
    }

    return true;
}
