#ifndef PETACTION_H
#define PETACTION_H

#include <QSize>
#include <QString>
#include <QStringList>

struct PetAction
{
    QString id;
    QString name;
    QString folderPath;
    int fps;
    int frameCount;
    QSize frameSize;
    QStringList frameFiles;
    bool enabled;

    PetAction();
    PetAction(const QString &actionId, const QString &actionName, const QString &path, int actionFps = 12, int frames = 0);

    bool isValid() const;
};

#endif // PETACTION_H
