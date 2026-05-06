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

    static ActionLibraryOperationResult deleteAction(
        const QString &petDir,
        QList<PetAction> actions,
        PetPlaylist playlist,
        const QString &actionId
    );
};

#endif // ACTIONLIBRARYSERVICE_H
