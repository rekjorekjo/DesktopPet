#ifndef TEST_CHATLOGSERVICE_H
#define TEST_CHATLOGSERVICE_H

#include <QObject>
#include <QTemporaryDir>

class TestChatLogService : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    QString m_caseRoot;
    int m_caseCounter = 0;

private slots:
    void init();
    void cleanup();

    void appendAndLoadLog();
    void appendMultipleEntries();
    void emptyPetIdUsesNoPet();
    void filterEntries();
    void formatEntriesForDisplay();
};

#endif // TEST_CHATLOGSERVICE_H
