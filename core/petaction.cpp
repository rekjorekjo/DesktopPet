#include "petaction.h"

PetAction::PetAction()
    : fps(15)
    , frameCount(0)
{
}

PetAction::PetAction(const QString &actionId, const QString &actionName, const QString &path, int actionFps, int frames)
    : id(actionId)
    , name(actionName)
    , folderPath(path)
    , fps(actionFps)
    , frameCount(frames)
{
}

bool PetAction::isValid() const
{
    return !id.isEmpty() && !folderPath.isEmpty();
}
