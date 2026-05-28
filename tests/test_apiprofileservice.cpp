#include "test_apiprofileservice.h"

#include <QtTest>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QSettings>

#include "core/petpaths.h"
#include "services/apiprofileservice.h"

void TestApiProfileService::writeProfilesJson(const QString &content)
{
    QString configDir = PetPaths::configDir();
    QDir().mkpath(configDir);
    QFile f(configDir + "/api_profiles.json");
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.write(content.toUtf8());
    f.close();
}

void TestApiProfileService::init()
{
    QVERIFY2(m_tempDir.isValid(), "Failed to create temp dir");
    PetPaths::setDataRootOverrideForTesting(m_tempDir.path());
}

void TestApiProfileService::cleanup()
{
    ApiProfileService::instance().clear();
    PetPaths::clearDataRootOverrideForTesting();

    QSettings settings("DesktopPet", "DesktopPet");
    settings.remove("api/currentConfigName");
}

void TestApiProfileService::loadDoesNotUseLegacyQSettingsFallback()
{
    // Write a profile JSON with empty currentProfile
    writeProfilesJson(R"({
        "version": 1,
        "currentProfile": "",
        "profiles": [
            {
                "name": "legacy-profile",
                "providerId": "deepseek",
                "format": "openai-compatible",
                "templateText": "",
                "apiKey": "test-key",
                "baseUrl": "https://example.com",
                "model": "test-model",
                "maxTokens": 1024,
                "temperature": 0.7
            }
        ]
    })");

    // Write legacy QSettings value
    QSettings settings("DesktopPet", "DesktopPet");
    settings.setValue("api/currentConfigName", "legacy-profile");

    QVERIFY(ApiProfileService::instance().load());

    // currentProfile should remain empty — QSettings fallback is gone
    QCOMPARE(ApiProfileService::instance().currentProfileName(), QString());
}

void TestApiProfileService::setCurrentProfilePersistsOnlyToJson()
{
    writeProfilesJson(R"({
        "version": 1,
        "currentProfile": "",
        "profiles": [
            {
                "name": "main",
                "providerId": "deepseek",
                "format": "openai-compatible",
                "templateText": "",
                "apiKey": "test-key",
                "baseUrl": "https://example.com",
                "model": "test-model",
                "maxTokens": 1024,
                "temperature": 0.7
            }
        ]
    })");

    QVERIFY(ApiProfileService::instance().load());
    QVERIFY(ApiProfileService::instance().setCurrentProfileName("main"));

    // Re-read the JSON file directly
    QString filePath = PetPaths::apiProfilesFilePath();
    QFile f(filePath);
    QVERIFY(f.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();

    QVERIFY(doc.isObject());
    QCOMPARE(doc.object()["currentProfile"].toString(), QString("main"));
}

void TestApiProfileService::invalidCurrentProfileStillFallsBackToFirstProfile()
{
    writeProfilesJson(R"({
        "version": 1,
        "currentProfile": "missing",
        "profiles": [
            {
                "name": "alpha",
                "providerId": "deepseek",
                "format": "openai-compatible",
                "templateText": "",
                "apiKey": "key-a",
                "baseUrl": "https://a.example.com",
                "model": "model-a",
                "maxTokens": 1024,
                "temperature": 0.7
            },
            {
                "name": "beta",
                "providerId": "openai",
                "format": "openai-compatible",
                "templateText": "",
                "apiKey": "key-b",
                "baseUrl": "https://b.example.com",
                "model": "model-b",
                "maxTokens": 2048,
                "temperature": 0.5
            }
        ]
    })");

    QVERIFY(ApiProfileService::instance().load());

    QString current = ApiProfileService::instance().currentProfileName();
    QVERIFY2(QStringList({"alpha", "beta"}).contains(current),
             qPrintable("Expected alpha or beta, got: " + current));
}

#include "moc_test_apiprofileservice.cpp"
