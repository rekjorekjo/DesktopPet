#ifndef TEST_UPDATESERVICE_H
#define TEST_UPDATESERVICE_H

#include <QObject>

class TestUpdateService : public QObject
{
    Q_OBJECT

private slots:
    void testParseReleaseJsonBasic();
    void testParseReleaseJsonSelectsInstallerAsset();
    void testParseReleaseJsonIgnoresSourceArchives();
    void testParseReleaseJsonSkipsPrerelease();
    void testParseReleaseJsonParsesDigest();
    void testParseReleaseJsonNoUpdateWhenCurrent();
    void testParseReleaseJsonNoUpdateWhenCurrentTagMatches();

    void testParseManifestJsonValid();
    void testParseManifestJsonMissingVersion();
    void testParseManifestJsonMissingInstallerUrl();
    void testParseManifestJsonInstallerUrlMismatch();
    void testParseManifestJsonInstallerUrlWithPathMismatch();
    void testParseManifestJsonNoUpdateWhenCurrent();
    void testParseManifestJsonNotesIgnored();
};

#endif // TEST_UPDATESERVICE_H
