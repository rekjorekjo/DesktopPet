#ifndef PETACTION_H
#define PETACTION_H

#include <QString>

struct PetAction
{
    QString id;
    QString name;
    QString folderPath;
    int fps;
    int frameCount;

    PetAction();
    PetAction(const QString &actionId, const QString &actionName, const QString &path, int actionFps = 15, int frames = 0);

    bool isValid() const;
};

#endif // PETACTION_H
