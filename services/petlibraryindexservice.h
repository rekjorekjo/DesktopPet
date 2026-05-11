#ifndef PETLIBRARYINDEXSERVICE_H
#define PETLIBRARYINDEXSERVICE_H

#include <QList>
#include <QString>
#include <optional>

// 宠物库索引条目
struct PetLibraryEntry
{
    QString id;       // 宠物唯一标识
    QString name;     // 宠物显示名称
    QString dir;      // 宠物目录名（默认与 id 相同）
    bool enabled = true;
};

// petlibrary.json 的索引服务
//
// petlibrary.json 是宠物 ID 是否存在的唯一依据。
// 目录存在不代表 ID 已占用，必须检查 library entry。
//
// 移除 vs 删除语义：
// - removePetEntry: 只删 library entry，不删目录
// - deletePetEntryAndDirectory: 删 library entry + 删目录
//
// 这样设计是为了支持"移除宠物但保留资源"的场景。
class PetLibraryIndexService
{
public:
    static QString libraryFilePath();

    // 确保宠物库索引文件存在
    static bool ensureLibrary();

    static QList<PetLibraryEntry> loadEntries();
    static bool saveEntries(const QList<PetLibraryEntry> &entries);

    // 检查宠物 ID 是否存在于 library 中
    // 注意：只检查 library entry，不检查目录是否存在
    static bool containsPetId(const QString &petId);
    static std::optional<PetLibraryEntry> findPet(const QString &petId);

    static bool addOrUpdatePet(const PetLibraryEntry &entry);

    // 从 library 中移除宠物条目（不删除目录）
    static bool removePetEntry(const QString &petId);

    // 从 library 中移除宠物条目并删除目录
    static bool deletePetEntryAndDirectory(const QString &petId);

    static QString findFirstEnabledPetId(const QString &excludePetId = QString());

private:
    static QList<PetLibraryEntry> scanExistingPets();
};

#endif // PETLIBRARYINDEXSERVICE_H
