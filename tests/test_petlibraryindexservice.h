#ifndef TEST_PETLIBRARYINDEXSERVICE_H
#define TEST_PETLIBRARYINDEXSERVICE_H

#include <QObject>
#include <QTemporaryDir>

class TestPetLibraryIndexService : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    QString m_caseRoot;
    int m_caseCounter = 0;

    void writePetJson(const QString &petDir, const QString &id, const QString &name = QString());
    void writePlaylistJson(const QString &petDir);
    void createValidPet(const QString &petId);

private slots:
    void init();
    void cleanup();

    void noPetsDirectory();
    void invalidDirectoryNotRecovered();
    void validDirectoryRecovered();
    void mismatchedIdSkipped();
    void existingLibraryNotOverwritten();
};

#endif
