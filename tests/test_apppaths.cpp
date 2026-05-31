#include "test_apppaths.h"

#include "core/apppaths.h"
#include "core/petpaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>
#include <QTest>

void TestAppPaths::appDirMatchesApplicationDirPath()
{
    QCOMPARE(AppPaths::appDir(), QCoreApplication::applicationDirPath());
}

void TestAppPaths::dataRootDirBasedOnAppDir()
{
    QString expected = AppPaths::appDir() + "/pets";
    QCOMPARE(AppPaths::dataRootDir(), expected);
}

void TestAppPaths::configDirBasedOnDataRoot()
{
    QString expected = AppPaths::dataRootDir() + "/config";
    QCOMPARE(AppPaths::configDir(), expected);
}

void TestAppPaths::petsDirBasedOnDataRoot()
{
    QString expected = AppPaths::dataRootDir() + "/pets";
    QCOMPARE(AppPaths::petsDir(), expected);
}

void TestAppPaths::actionsDirBasedOnDataRoot()
{
    QString expected = AppPaths::dataRootDir() + "/actions";
    QCOMPARE(AppPaths::actionsDir(), expected);
}

void TestAppPaths::logsDirBasedOnDataRoot()
{
    QString expected = AppPaths::dataRootDir() + "/logs";
    QCOMPARE(AppPaths::logsDir(), expected);
}

void TestAppPaths::chatLogsDirBasedOnDataRoot()
{
    QString expected = AppPaths::dataRootDir() + "/logs/chat";
    QCOMPARE(AppPaths::chatLogsDir(), expected);
}

void TestAppPaths::petLibraryFileBasedOnDataRoot()
{
    QString expected = AppPaths::dataRootDir() + "/petlibrary.json";
    QCOMPARE(AppPaths::petLibraryFile(), expected);
}

void TestAppPaths::resizeToolPathBasedOnAppDir()
{
    QString path = AppPaths::resizeToolPath();
    QVERIFY(path.startsWith(AppPaths::appDir()));
    QVERIFY(path.contains("DesktopPet-resize"));
}

void TestAppPaths::corePathsDoNotDependOnCurrentPath()
{
    // Verify that all derived paths are under dataRootDir, not QDir::currentPath().
    QString dataRoot = AppPaths::dataRootDir();

    QVERIFY(AppPaths::configDir().startsWith(dataRoot));
    QVERIFY(AppPaths::petsDir().startsWith(dataRoot));
    QVERIFY(AppPaths::actionsDir().startsWith(dataRoot));
    QVERIFY(AppPaths::logsDir().startsWith(dataRoot));
    QVERIFY(AppPaths::chatLogsDir().startsWith(dataRoot));
    QVERIFY(AppPaths::petLibraryFile().startsWith(dataRoot));
}

void TestAppPaths::petPathsMatchesAppPaths()
{
    // Without override, PetPaths should delegate to AppPaths.
    PetPaths::clearDataRootOverrideForTesting();

    QCOMPARE(PetPaths::rootDirectory(), AppPaths::dataRootDir());
    QCOMPARE(PetPaths::configDir(), AppPaths::configDir());
    QCOMPARE(PetPaths::petsDirectory(), AppPaths::petsDir());
    QCOMPARE(PetPaths::actionsDirectory(), AppPaths::actionsDir());
}

void TestAppPaths::petPathsOverrideIsRespected()
{
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());

    PetPaths::setDataRootOverrideForTesting(tempDir.path());

    QCOMPARE(PetPaths::rootDirectory(), tempDir.path());
    QCOMPARE(PetPaths::configDir(), tempDir.path() + "/config");
    QCOMPARE(PetPaths::petsDirectory(), tempDir.path() + "/pets");
    QCOMPARE(PetPaths::actionsDirectory(), tempDir.path() + "/actions");

    PetPaths::clearDataRootOverrideForTesting();
}

#include "moc_test_apppaths.cpp"
