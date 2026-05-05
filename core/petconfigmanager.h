#ifndef PETCONFIGMANAGER_H
#define PETCONFIGMANAGER_H

#include <QJsonObject>
#include <QList>
#include <QSize>
#include <QString>

#include "core/petaction.h"
#include "core/petplaylist.h"

struct PetBasicInfo
{
    QString id;
    QString name;
    QSize canvasSize;
    QSize displaySize;

    PetBasicInfo();
};

class PetConfigManager
{
public:
    static bool loadPetJson(const QString &filePath, PetBasicInfo &info, QList<PetAction> &actions);
    static bool savePetJson(const QString &filePath, const PetBasicInfo &info, const QList<PetAction> &actions);

    static bool loadPlaylistFromJson(const QString &filePath, PetPlaylist &playlist);
    static bool savePlaylistToJson(const QString &filePath, const PetPlaylist &playlist);

    static bool loadPetFromDirectory(const QString &petDirPath, PetBasicInfo &info, QList<PetAction> &actions, PetPlaylist &playlist);

private:
    static QJsonObject actionToJson(const PetAction &action);
    static PetAction jsonToAction(const QJsonObject &obj);

    static QJsonObject actionRefToJson(const PetActionRef &ref);
    static PetActionRef jsonToActionRef(const QJsonObject &obj);

    static void scanActionFrames(PetAction &action);
    static QStringList scanFrameFiles(const QString &folderPath);
    static bool naturalSortLessThan(const QString &a, const QString &b);
};

#endif // PETCONFIGMANAGER_H
