#ifndef TEST_REPOSITORYCLEANUP_H
#define TEST_REPOSITORYCLEANUP_H

#include <QObject>
#include <QTemporaryDir>

class TestRepositoryCleanup : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;

private slots:
    void init();
    void cleanup();

    void petPathsNoDefaultPet();
    void chatLogServiceNoDefaultPet();
};

#endif // TEST_REPOSITORYCLEANUP_H
