#include "test_petlibraryindexservice.h"

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "core/appsettings.h"
#include "core/petpaths.h"
#include "services/petlibraryindexservice.h"

void TestPetLibraryIndexService::writePetJson(const QString &petDir, const QString &id, const QString &name)
{
    QJsonObject obj;
    obj["id"] = id;
    obj["name"] = name.isEmpty() ? id : name;
    QJsonObject canvas;
    canvas["width"] = 400;
    canvas["height"] = 400;
    obj["canvasSize"] = canvas;
    QJsonObject display;
    display["width"] = 200;
    display["height"] = 200;
    obj["displaySize"] = display;
    obj["enabled"] = true;

    QFile f(petDir + "/pet.json");
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    f.close();
}

void TestPetLibraryIndexService::writePlaylistJson(const QString &petDir)
{
    QJsonObject obj;
    obj["idle"] = QJsonArray();
    obj["random"] = QJsonArray();
    obj["timed"] = QJsonArray();
    obj["emotion"] = QJsonObject();

    QFile f(petDir + "/playlist.json");
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    f.close();
}

void TestPetLibraryIndexService::createValidPet(const QString &petId)
{
    QString petDir = PetPaths::petDirectory(petId);
    QDir().mkpath(petDir);
    writePetJson(petDir, petId);
    writePlaylistJson(petDir);
}

void TestPetLibraryIndexService::init()
{
    QVERIFY2(m_tempDir.isValid(), "Failed to create temp dir");

    QString caseName = QString::fromLatin1(QTest::currentTestFunction());
    m_caseRoot = m_tempDir.path() + "/" + caseName + "_" + QString::number(++m_caseCounter);
    QVERIFY(QDir().mkpath(m_caseRoot));

    PetPaths::setDataRootOverrideForTesting(m_caseRoot);
    AppSettings::setCurrentPetId(QString());
}

void TestPetLibraryIndexService::cleanup()
{
    PetPaths::clearDataRootOverrideForTesting();
    AppSettings::setCurrentPetId(QString());

    if (!m_caseRoot.isEmpty()) {
        QDir dir(m_caseRoot);
        if (dir.exists())
            dir.removeRecursively();
        m_caseRoot.clear();
    }
}

void TestPetLibraryIndexService::noPetsDirectory()
{
    bool recovered = PetLibraryIndexService::recoverLibraryFromDiskIfEmpty();
    QCOMPARE(recovered, false);

    auto entries = PetLibraryIndexService::loadEntries();
    QCOMPARE(entries.size(), 0);

    QVERIFY(AppSettings::currentPetId().isEmpty());
}

void TestPetLibraryIndexService::invalidDirectoryNotRecovered()
{
    QString petsDir = PetPaths::petsDirectory();
    QDir().mkpath(petsDir + "/bad_pet");

    bool recovered = PetLibraryIndexService::recoverLibraryFromDiskIfEmpty();
    QCOMPARE(recovered, false);

    auto entries = PetLibraryIndexService::loadEntries();
    QCOMPARE(entries.size(), 0);
}

void TestPetLibraryIndexService::validDirectoryRecovered()
{
    createValidPet("cat");

    bool recovered = PetLibraryIndexService::recoverLibraryFromDiskIfEmpty();
    QCOMPARE(recovered, true);

    auto entries = PetLibraryIndexService::loadEntries();
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.first().id, QString("cat"));

    QCOMPARE(AppSettings::currentPetId(), QString("cat"));
}

void TestPetLibraryIndexService::mismatchedIdSkipped()
{
    QString petDir = PetPaths::petDirectory("folderA");
    QDir().mkpath(petDir);
    writePetJson(petDir, "petB");
    writePlaylistJson(petDir);

    bool recovered = PetLibraryIndexService::recoverLibraryFromDiskIfEmpty();
    QCOMPARE(recovered, false);

    auto entries = PetLibraryIndexService::loadEntries();
    QCOMPARE(entries.size(), 0);
}

void TestPetLibraryIndexService::existingLibraryNotOverwritten()
{
    // Step 1: Create and recover "cat"
    createValidPet("cat");
    PetLibraryIndexService::recoverLibraryFromDiskIfEmpty();

    auto entries = PetLibraryIndexService::loadEntries();
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.first().id, QString("cat"));

    // Step 2: Create "dog" on disk but don't add to library
    createValidPet("dog");

    // Step 3: Recovery should NOT add dog
    bool recovered = PetLibraryIndexService::recoverLibraryFromDiskIfEmpty();
    QCOMPARE(recovered, false);

    entries = PetLibraryIndexService::loadEntries();
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.first().id, QString("cat"));
}

#include "moc_test_petlibraryindexservice.cpp"
