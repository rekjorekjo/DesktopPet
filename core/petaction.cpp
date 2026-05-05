#include "petaction.h"

PetAction::PetAction()
    : fps(15)
    , frameCount(0)
    , enabled(true)
{
}

PetAction::PetAction(const QString &actionId, const QString &actionName, const QString &path, int actionFps, int frames)
    : id(actionId)
    , name(actionName)
    , folderPath(path)
    , fps(actionFps)
    , frameCount(frames)
    , enabled(true)
{
}

bool PetAction::isValid() const
{
    return !id.isEmpty() && !folderPath.isEmpty();
}
