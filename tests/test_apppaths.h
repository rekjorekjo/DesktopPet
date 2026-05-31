#ifndef TEST_APPPATHS_H
#define TEST_APPPATHS_H

#include <QObject>

class TestAppPaths : public QObject
{
    Q_OBJECT

private slots:
    void appDirMatchesApplicationDirPath();
    void dataRootDirBasedOnAppDir();
    void configDirBasedOnDataRoot();
    void petsDirBasedOnDataRoot();
    void actionsDirBasedOnDataRoot();
    void logsDirBasedOnDataRoot();
    void chatLogsDirBasedOnDataRoot();
    void petLibraryFileBasedOnDataRoot();
    void resizeToolPathBasedOnAppDir();
    void corePathsDoNotDependOnCurrentPath();

    // PetPaths integration: no override
    void petPathsMatchesAppPaths();
    // PetPaths integration: with override
    void petPathsOverrideIsRespected();
};

#endif // TEST_APPPATHS_H
