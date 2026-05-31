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
