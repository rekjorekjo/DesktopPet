#include "test_secretstorageservice.h"

#include <QtTest>
#include "services/secretstorageservice.h"

void TestSecretStorageService::backendName()
{
    QCOMPARE(SecretStorageService::backendName(), QStringLiteral("Plain JSON"));
}

void TestSecretStorageService::usesSecureStorage()
{
    QCOMPARE(SecretStorageService::usesSecureStorage(), false);
}

void TestSecretStorageService::maskSecret_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("empty")       << QString()       << QString();
    QTest::newRow("short")       << "12345678"      << "••••";
    QTest::newRow("sk-prefix")   << "sk-1234567890abcdef" << "sk-1••••cdef";
}

void TestSecretStorageService::maskSecret()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QCOMPARE(SecretStorageService::maskSecret(input), expected);
}

void TestSecretStorageService::looksLikeSecret_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<bool>("expected");

    QTest::newRow("sk-prefix")       << "sk-xxx"                << true;
    QTest::newRow("bearer")          << "Bearer abc"            << true;
    QTest::newRow("api_key")         << "api_key=abc"           << true;
    QTest::newRow("apikey")          << "apikey=abc"            << true;
    QTest::newRow("authorization")   << "authorization: Bearer abc" << true;
    QTest::newRow("normal-chinese")  << "普通错误信息"           << false;
    QTest::newRow("empty")           << QString()               << false;
}

void TestSecretStorageService::looksLikeSecret()
{
    QFETCH(QString, input);
    QFETCH(bool, expected);
    QCOMPARE(SecretStorageService::looksLikeSecret(input), expected);
}

void TestSecretStorageService::redactSecrets_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("forbidden");
    QTest::addColumn<bool>("shouldContainRedacted");

    QTest::newRow("bearer-token")
        << "Authorization: Bearer abcdef"
        << "abcdef"
        << true;

    QTest::newRow("sk-key")
        << "sk-1234567890"
        << "sk-1234567890"
        << true;

    QTest::newRow("normal-message")
        << "连接超时，请检查网络"
        << ""
        << false;
}

void TestSecretStorageService::redactSecrets()
{
    QFETCH(QString, input);
    QFETCH(QString, forbidden);
    QFETCH(bool, shouldContainRedacted);

    QString result = SecretStorageService::redactSecrets(input);

    if (shouldContainRedacted) {
        QVERIFY2(!result.contains(forbidden),
                 qPrintable(QString("Redacted result still contains secret: '%1'\nResult: '%2'")
                                .arg(forbidden, result)));
        QVERIFY(result.contains("[REDACTED"));
    } else {
        QVERIFY(!result.isEmpty() || input.isEmpty());
    }
}

#include "moc_test_secretstorageservice.cpp"
