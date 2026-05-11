#ifndef PETLIBRARYSERVICE_H
#define PETLIBRARYSERVICE_H

#include "services/petlibraryindexservice.h"
#include <QString>

struct PetLibraryOperationResult
{
    bool success = false;
    bool warning = false;
    QString message;
    QString nextCurrentPetId;
};

class PetLibraryService
{
public:
    // "移除宠物"
    // 只从 petlibrary.json 删除 entry
    // 不删除宠物目录
    // 目录保留只是资源残留，不再占用 petId
    static PetLibraryOperationResult disablePet(const QString &petId);

    // "删除宠物"
    // 从 petlibrary.json 删除 entry
    // 删除宠物目录
    // 如果目录不存在，也视为删除成功 / 等同移除
    static PetLibraryOperationResult deletePet(const QString &petId);

private:
    static QString findFirstEnabledPetId(const QString &excludePetId);
    static int countEnabledPets(const QString &excludePetId = QString());
};

#endif
