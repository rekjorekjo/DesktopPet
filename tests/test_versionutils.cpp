#include "test_versionutils.h"
#include "core/versionutils.h"

#include <QTest>

void TestVersionUtils::testEqualVersions()
{
    QCOMPARE(VersionUtils::compareVersions("1.0.0", "1.0.0"), 0);
    QCOMPARE(VersionUtils::compareVersions("v1.0.0", "1.0.0"), 0);
    QCOMPARE(VersionUtils::compareVersions("v1.0.0", "v1.0.0"), 0);
}

void TestVersionUtils::testMajorVersionDifference()
{
    QVERIFY(VersionUtils::compareVersions("2.0.0", "1.0.0") > 0);
    QVERIFY(VersionUtils::compareVersions("1.0.0", "2.0.0") < 0);
    QVERIFY(VersionUtils::compareVersions("1.0.0", "0.32.4") > 0);
}

void TestVersionUtils::testMinorVersionDifference()
{
    QVERIFY(VersionUtils::compareVersions("1.1.0", "1.0.9") > 0);
    QVERIFY(VersionUtils::compareVersions("1.0.9", "1.1.0") < 0);
}

void TestVersionUtils::testPatchVersionDifference()
{
    QVERIFY(VersionUtils::compareVersions("1.0.1", "1.0.0") > 0);
    QVERIFY(VersionUtils::compareVersions("0.32.4", "0.32.3") > 0);
}

void TestVersionUtils::testPreReleaseVsRelease()
{
    QVERIFY(VersionUtils::compareVersions("1.0.0-beta", "1.0.0") < 0);
    QVERIFY(VersionUtils::compareVersions("1.0.0", "1.0.0-beta") > 0);
}

void TestVersionUtils::testVersionWithVPrefix()
{
    QCOMPARE(VersionUtils::compareVersions("v1.0.0", "1.0.0"), 0);
    QVERIFY(VersionUtils::compareVersions("v1.0.1", "v1.0.0") > 0);
}

void TestVersionUtils::testZeroVersion()
{
    QCOMPARE(VersionUtils::compareVersions("0.0.0", "0.0.0"), 0);
    QVERIFY(VersionUtils::compareVersions("0.0.1", "0.0.0") > 0);
}
