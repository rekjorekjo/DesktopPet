#include "test_repositorycleanup.h"

#include <QtTest>
#include <QDir>

#include "core/petpaths.h"
#include "services/chatlogservice.h"

void TestRepositoryCleanup::init()
{
    QVERIFY2(m_tempDir.isValid(), "Failed to create temp dir");
    PetPaths::setDataRootOverrideForTesting(m_tempDir.path());
}

void TestRepositoryCleanup::cleanup()
{
    PetPaths::clearDataRootOverrideForTesting();
}

void TestRepositoryCleanup::petPathsNoDefaultPet()
{
    PetPaths::ensureDefaultStructure();

    QVERIFY(!QDir(m_tempDir.path() + "/pets/default_pet").exists());
    QVERIFY(!QDir(m_tempDir.path() + "/default_pet").exists());
}

void TestRepositoryCleanup::chatLogServiceNoDefaultPet()
{
    QString dir = ChatLogService::petLogDirectory(QString());
    QVERIFY2(!dir.contains("default_pet"),
             qPrintable("Log directory should not contain default_pet: " + dir));
    QVERIFY2(dir.contains("_no_pet"),
             qPrintable("Log directory should contain _no_pet: " + dir));
}

#include "moc_test_repositorycleanup.cpp"
