#ifndef ACTIONLIBRARYINDEXSERVICE_H
#define ACTIONLIBRARYINDEXSERVICE_H

#include <QList>
#include <QString>
#include <optional>

struct ActionLibraryEntry
{
    QString id;
    QString name;
    QString dir;
};

class ActionLibraryIndexService
{
public:
    static QString libraryFilePath();

    static bool ensureLibrary();

    static QList<ActionLibraryEntry> loadEntries();
    static bool saveEntries(const QList<ActionLibraryEntry> &entries);

    static QList<ActionLibraryEntry> loadEntriesFromFile(const QString &filePath);

    static bool containsActionId(const QString &actionId);
    static std::optional<ActionLibraryEntry> findAction(const QString &actionId);

    static bool addOrUpdateAction(const ActionLibraryEntry &entry);
    static bool removeActionEntry(const QString &actionId);
    static bool deleteActionEntryAndDirectory(const QString &actionId);

private:
    static QList<ActionLibraryEntry> scanExistingActions();
};

#endif // ACTIONLIBRARYINDEXSERVICE_H
