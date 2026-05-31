#ifndef TEST_VERSIONUTILS_H
#define TEST_VERSIONUTILS_H

#include <QObject>

class TestVersionUtils : public QObject
{
    Q_OBJECT

private slots:
    void testEqualVersions();
    void testMajorVersionDifference();
    void testMinorVersionDifference();
    void testPatchVersionDifference();
    void testPreReleaseVsRelease();
    void testVersionWithVPrefix();
    void testZeroVersion();
};

#endif // TEST_VERSIONUTILS_H
