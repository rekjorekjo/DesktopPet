#ifndef ACTIONLIBRARYSERVICE_H
#define ACTIONLIBRARYSERVICE_H

#include <QList>
#include <QString>

#include "models/petaction.h"
#include "models/petplaylist.h"

struct ActionLibraryOperationResult
{
    bool success = false;
    bool warning = false;
    QString message;
    int removedReferenceCount = 0;
    int cleanedPetCount = 0;
    int failedPlaylistCount = 0;
    bool actionDirectoryDeleted = false;
    int replacedReferenceCount = 0;
};

class ActionLibraryService
{
public:
    static ActionLibraryOperationResult disableAction(
        const QString &petDir,
        QList<PetAction> actions,
        PetPlaylist playlist,
        const QString &actionId
    );

    static ActionLibraryOperationResult removeAction(
        const QString &actionId
    );

    static ActionLibraryOperationResult deleteAction(
        const QString &actionId
    );

    static ActionLibraryOperationResult deleteAction(
        const QString &petDir,
        QList<PetAction> actions,
        PetPlaylist playlist,
        const QString &actionId
    );

    static ActionLibraryOperationResult renameActionId(
        const QString &oldActionId,
        const QString &newActionId
    );
};

#endif // ACTIONLIBRARYSERVICE_H
