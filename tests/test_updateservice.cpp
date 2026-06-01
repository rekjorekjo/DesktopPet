#include "test_updateservice.h"
#include "services/updateservice.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

void TestUpdateService::testParseReleaseJsonBasic()
{
    QJsonObject release;
    release["tag_name"] = "v1.0.1";
    release["name"] = "Release 1.0.1";
    release["body"] = "Bug fixes";
    release["html_url"] = "https://github.com/rekjorekjo/DesktopPet/releases/tag/v1.0.1";
    release["draft"] = false;
    release["prerelease"] = false;
    release["assets"] = QJsonArray();

    UpdateInfo info = UpdateService::parseReleaseJson(release, "1.0.0");

    QVERIFY(info.valid);
    QVERIFY(info.updateAvailable);
    QCOMPARE(info.latestVersion, QString("v1.0.1"));
    QCOMPARE(info.releaseName, QString("Release 1.0.1"));
    QCOMPARE(info.releaseNotes, QString("Bug fixes"));
    QCOMPARE(info.htmlUrl, QString("https://github.com/rekjorekjo/DesktopPet/releases/tag/v1.0.1"));
}

void TestUpdateService::testParseReleaseJsonSelectsInstallerAsset()
{
    QJsonObject asset;
    asset["name"] = "DesktopPet_Setup_v1.0.1.exe";
    asset["browser_download_url"] = "https://github.com/rekjorekjo/DesktopPet/releases/download/v1.0.1/DesktopPet_Setup_v1.0.1.exe";
    asset["size"] = 12345678;

    QJsonArray assets;
    assets.append(asset);

    QJsonObject release;
    release["tag_name"] = "v1.0.1";
    release["name"] = "Release 1.0.1";
    release["body"] = "";
    release["html_url"] = "https://example.com";
    release["draft"] = false;
    release["prerelease"] = false;
    release["assets"] = assets;

    UpdateInfo info = UpdateService::parseReleaseJson(release, "1.0.0");

    QVERIFY(info.valid);
    QVERIFY(info.updateAvailable);
    QCOMPARE(info.assetName, QString("DesktopPet_Setup_v1.0.1.exe"));
    QCOMPARE(info.downloadUrl, QString("https://github.com/rekjorekjo/DesktopPet/releases/download/v1.0.1/DesktopPet_Setup_v1.0.1.exe"));
    QCOMPARE(info.assetSize, 12345678LL);
}

void TestUpdateService::testParseReleaseJsonIgnoresSourceArchives()
{
    QJsonObject installer;
    installer["name"] = "DesktopPet_Setup_v1.0.1.exe";
    installer["browser_download_url"] = "https://example.com/setup.exe";
    installer["size"] = 1000;

    QJsonObject zipball;
    zipball["name"] = "Source code (zip)";
    zipball["browser_download_url"] = "https://example.com/source.zip";
    zipball["size"] = 500;

    QJsonObject tarball;
    tarball["name"] = "Source code (tar.gz)";
    tarball["browser_download_url"] = "https://example.com/source.tar.gz";
    tarball["size"] = 400;

    QJsonArray assets;
    assets.append(zipball);
    assets.append(tarball);
    assets.append(installer);

    QJsonObject release;
    release["tag_name"] = "v1.0.1";
    release["name"] = "";
    release["body"] = "";
    release["html_url"] = "https://example.com";
    release["draft"] = false;
    release["prerelease"] = false;
    release["assets"] = assets;

    UpdateInfo info = UpdateService::parseReleaseJson(release, "1.0.0");

    QCOMPARE(info.assetName, QString("DesktopPet_Setup_v1.0.1.exe"));
    QCOMPARE(info.downloadUrl, QString("https://example.com/setup.exe"));
}

void TestUpdateService::testParseReleaseJsonSkipsPrerelease()
{
    QJsonObject release;
    release["tag_name"] = "v1.1.0-beta";
    release["name"] = "Beta";
    release["body"] = "";
    release["html_url"] = "https://example.com";
    release["draft"] = false;
    release["prerelease"] = true;
    release["assets"] = QJsonArray();

    UpdateInfo info = UpdateService::parseReleaseJson(release, "1.0.0");

    QVERIFY(!info.valid);
    QVERIFY(!info.updateAvailable);
}

void TestUpdateService::testParseReleaseJsonParsesDigest()
{
    QJsonObject asset;
    asset["name"] = "DesktopPet_Setup_v1.0.1.exe";
    asset["browser_download_url"] = "https://example.com/setup.exe";
    asset["size"] = 1000;
    asset["digest"] = "sha256:abc123def456";

    QJsonArray assets;
    assets.append(asset);

    QJsonObject release;
    release["tag_name"] = "v1.0.1";
    release["name"] = "";
    release["body"] = "";
    release["html_url"] = "https://example.com";
    release["draft"] = false;
    release["prerelease"] = false;
    release["assets"] = assets;

    UpdateInfo info = UpdateService::parseReleaseJson(release, "1.0.0");

    QCOMPARE(info.sha256, QString("abc123def456"));
}

void TestUpdateService::testParseReleaseJsonNoUpdateWhenCurrent()
{
    QJsonObject release;
    release["tag_name"] = "v1.0.0";
    release["name"] = "Release 1.0.0";
    release["body"] = "";
    release["html_url"] = "https://example.com";
    release["draft"] = false;
    release["prerelease"] = false;
    release["assets"] = QJsonArray();

    UpdateInfo info = UpdateService::parseReleaseJson(release, "1.0.0");

    QVERIFY(info.valid);
    QVERIFY(!info.updateAvailable);
}

void TestUpdateService::testParseReleaseJsonNoUpdateWhenCurrentTagMatches()
{
    QJsonObject release;
    release["tag_name"] = "v0.32.4";
    release["name"] = "Release 0.32.4";
    release["body"] = "";
    release["html_url"] = "https://example.com";
    release["draft"] = false;
    release["prerelease"] = false;
    release["assets"] = QJsonArray();

    UpdateInfo info = UpdateService::parseReleaseJson(release, "v0.32.4");

    QVERIFY(info.valid);
    QVERIFY(!info.updateAvailable);
}

void TestUpdateService::testParseManifestJsonValid()
{
    QJsonObject manifest;
    manifest["version"] = "1.1.1";
    manifest["tag"] = "v1.1.1";
    manifest["releaseUrl"] = "https://github.com/rekjorekjo/DesktopPet/releases/tag/v1.1.1";
    manifest["installerUrl"] = "https://github.com/rekjorekjo/DesktopPet/releases/download/v1.1.1/DesktopPet_Setup_v1.1.1.exe";
    manifest["installerName"] = "DesktopPet_Setup_v1.1.1.exe";
    manifest["publishedAt"] = "2026-05-31T00:00:00Z";
    manifest["notes"] = "稳定性修复版本";

    UpdateInfo info = UpdateService::parseManifestJson(manifest, "1.1.0");

    QVERIFY(info.valid);
    QVERIFY(info.updateAvailable);
    QCOMPARE(info.latestVersion, QString("1.1.1"));
    QCOMPARE(info.downloadUrl, QString("https://github.com/rekjorekjo/DesktopPet/releases/download/v1.1.1/DesktopPet_Setup_v1.1.1.exe"));
    QCOMPARE(info.assetName, QString("DesktopPet_Setup_v1.1.1.exe"));
    QCOMPARE(info.htmlUrl, QString("https://github.com/rekjorekjo/DesktopPet/releases/tag/v1.1.1"));
    QVERIFY(info.releaseName.isEmpty());
    QVERIFY(info.releaseNotes.isEmpty());
}

void TestUpdateService::testParseManifestJsonMissingVersion()
{
    QJsonObject manifest;
    manifest["installerUrl"] = "https://github.com/rekjorekjo/DesktopPet/releases/download/v1.1.1/DesktopPet_Setup_v1.1.1.exe";

    UpdateInfo info = UpdateService::parseManifestJson(manifest, "1.1.0");

    QVERIFY(!info.valid);
    QVERIFY(!info.updateAvailable);
}

void TestUpdateService::testParseManifestJsonMissingInstallerUrl()
{
    QJsonObject manifest;
    manifest["version"] = "1.1.1";

    UpdateInfo info = UpdateService::parseManifestJson(manifest, "1.1.0");

    QVERIFY(!info.valid);
    QVERIFY(!info.updateAvailable);
}

void TestUpdateService::testParseManifestJsonInstallerUrlMismatch()
{
    QJsonObject manifest;
    manifest["version"] = "1.1.1";
    manifest["installerUrl"] = "https://github.com/rekjorekjo/DesktopPet/releases/download/v1.1.1/DesktopPet_Source_v1.1.1.exe";

    UpdateInfo info = UpdateService::parseManifestJson(manifest, "1.1.0");

    QVERIFY(!info.valid);
    QVERIFY(!info.updateAvailable);
}

void TestUpdateService::testParseManifestJsonInstallerUrlWithPathMismatch()
{
    // URL where "DesktopPet_Setup" appears in path but final filename is not DesktopPet_Setup*.exe
    QJsonObject manifest;
    manifest["version"] = "1.1.1";
    manifest["installerUrl"] = "https://example.com/DesktopPet_Setup/something.exe";

    UpdateInfo info = UpdateService::parseManifestJson(manifest, "1.1.0");

    QVERIFY(!info.valid);
    QVERIFY(!info.updateAvailable);
}

void TestUpdateService::testParseManifestJsonNoUpdateWhenCurrent()
{
    QJsonObject manifest;
    manifest["version"] = "1.1.0";
    manifest["installerUrl"] = "https://github.com/rekjorekjo/DesktopPet/releases/download/v1.1.0/DesktopPet_Setup_v1.1.0.exe";

    UpdateInfo info = UpdateService::parseManifestJson(manifest, "1.1.0");

    QVERIFY(info.valid);
    QVERIFY(!info.updateAvailable);
}

void TestUpdateService::testParseManifestJsonNotesIgnored()
{
    QJsonObject manifest;
    manifest["version"] = "1.1.1";
    manifest["installerUrl"] = "https://github.com/rekjorekjo/DesktopPet/releases/download/v1.1.1/DesktopPet_Setup_v1.1.1.exe";
    manifest["notes"] = "**Markdown** _should not_ be rendered as `code`";

    UpdateInfo info = UpdateService::parseManifestJson(manifest, "1.1.0");

    QVERIFY(info.valid);
    QVERIFY(info.updateAvailable);
    // Both releaseName and releaseNotes should be empty — notes are ignored
    QVERIFY(info.releaseName.isEmpty());
    QVERIFY(info.releaseNotes.isEmpty());
}
