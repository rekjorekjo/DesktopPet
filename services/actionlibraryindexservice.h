#ifndef ACTIONLIBRARYINDEXSERVICE_H
#define ACTIONLIBRARYINDEXSERVICE_H

#include <QList>
#include <QString>
#include <optional>

// 动作库索引条目
struct ActionLibraryEntry
{
    QString id;    // 动作唯一标识
    QString name;  // 动作显示名称
    QString dir;   // 动作目录名（默认与 id 相同）
};

// actionlibrary.json 的索引服务
//
// actionlibrary.json 是动作 ID 是否存在的唯一依据。
// actions/<actionId>/ 目录存在不代表 actionId 已占用，必须检查 library entry。
// 目录可能是移除动作后保留的资源，不能作为 ID 冲突判断依据。
//
// 移除 vs 删除语义：
// - removeActionEntry: 只删 library entry，不删文件夹
// - deleteActionEntryAndDirectory: 删 library entry + 删文件夹
//
// 这样设计是为了支持"移除动作但保留资源"的场景。
class ActionLibraryIndexService
{
public:
    static QString libraryFilePath();

    // 确保动作库目录和索引文件存在
    // 如果 pets/actions 目录不存在则创建
    // 如果 actionlibrary.json 不存在则创建空索引文件
    static bool ensureLibrary();

    static QList<ActionLibraryEntry> loadEntries();
    static bool saveEntries(const QList<ActionLibraryEntry> &entries);

    // 从指定文件加载动作库索引（用于导入动作库）
    static QList<ActionLibraryEntry> loadEntriesFromFile(const QString &filePath);

    // 检查动作 ID 是否存在于 library 中
    // 注意：只检查 library entry，不检查目录是否存在
    static bool containsActionId(const QString &actionId);
    static std::optional<ActionLibraryEntry> findAction(const QString &actionId);

    static bool addOrUpdateAction(const ActionLibraryEntry &entry);

    // 从 library 中移除动作条目（不删除文件夹）
    static bool removeActionEntry(const QString &actionId);

    // 从 library 中移除动作条目并删除文件夹
    static bool deleteActionEntryAndDirectory(const QString &actionId);

private:
    static QList<ActionLibraryEntry> scanExistingActions();
};

#endif // ACTIONLIBRARYINDEXSERVICE_H
