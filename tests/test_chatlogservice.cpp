#include "test_chatlogservice.h"

#include <QtTest>
#include <QDateTime>
#include <QDir>
#include <QFile>

#include "core/petpaths.h"
#include "services/chatlogservice.h"

void TestChatLogService::init()
{
    QVERIFY2(m_tempDir.isValid(), "Failed to create temp dir");

    QString caseName = QString::fromLatin1(QTest::currentTestFunction());
    m_caseRoot = m_tempDir.path() + "/" + caseName + "_" + QString::number(++m_caseCounter);
    QVERIFY(QDir().mkpath(m_caseRoot));

    PetPaths::setDataRootOverrideForTesting(m_caseRoot);
}

void TestChatLogService::cleanup()
{
    PetPaths::clearDataRootOverrideForTesting();

    if (!m_caseRoot.isEmpty()) {
        QDir dir(m_caseRoot);
        if (dir.exists()) {
            dir.removeRecursively();
        }
        m_caseRoot.clear();
    }
}

void TestChatLogService::appendAndLoadLog()
{
    ChatLogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.petId = "cat";
    entry.petName = "猫猫";
    entry.apiConfigName = "main";
    entry.provider = "deepseek";
    entry.model = "deepseek-chat";
    entry.role = "user";
    entry.content = "你好";
    entry.conversationId = "test-conversation";
    entry.error = false;

    QVERIFY(ChatLogService::appendLog(entry));

    QString filePath = ChatLogService::logFilePath("cat", entry.timestamp.date());
    QVERIFY(QFile::exists(filePath));

    QList<ChatLogEntry> entries = ChatLogService::loadLogFile(filePath);
    QCOMPARE(entries.size(), 1);
    QCOMPARE(entries.first().role, QString("user"));
    QCOMPARE(entries.first().content, QString("你好"));
    QCOMPARE(entries.first().petId, QString("cat"));
    QCOMPARE(entries.first().conversationId, QString("test-conversation"));
}

void TestChatLogService::appendMultipleEntries()
{
    QDateTime now = QDateTime::currentDateTime();

    ChatLogEntry userEntry;
    userEntry.timestamp = now;
    userEntry.petId = "cat";
    userEntry.petName = "猫猫";
    userEntry.role = "user";
    userEntry.content = "你好";
    userEntry.conversationId = "conv-1";
    QVERIFY(ChatLogService::appendLog(userEntry));

    ChatLogEntry assistantEntry;
    assistantEntry.timestamp = now;
    assistantEntry.petId = "cat";
    assistantEntry.petName = "猫猫";
    assistantEntry.role = "assistant";
    assistantEntry.content = "你好呀！";
    assistantEntry.conversationId = "conv-1";
    QVERIFY(ChatLogService::appendLog(assistantEntry));

    QString filePath = ChatLogService::logFilePath("cat", now.date());
    QList<ChatLogEntry> entries = ChatLogService::loadLogFile(filePath);
    QCOMPARE(entries.size(), 2);
    QCOMPARE(entries.at(0).role, QString("user"));
    QCOMPARE(entries.at(1).role, QString("assistant"));
}

void TestChatLogService::emptyPetIdUsesNoPet()
{
    ChatLogEntry entry;
    entry.timestamp = QDateTime::currentDateTime();
    entry.petId = "";
    entry.role = "user";
    entry.content = "test";
    entry.conversationId = "conv-no-pet";

    QVERIFY(ChatLogService::appendLog(entry));

    QString petDir = ChatLogService::petLogDirectory("");
    QVERIFY2(petDir.contains("_no_pet"),
             qPrintable("Expected _no_pet in path: " + petDir));
    QVERIFY2(!petDir.contains("default_pet"),
             qPrintable("Should not contain default_pet: " + petDir));
}

void TestChatLogService::filterEntries()
{
    QList<ChatLogEntry> entries;

    ChatLogEntry e1;
    e1.role = "user";
    e1.content = "今天天气怎么样";
    e1.provider = "deepseek";
    e1.model = "deepseek-chat";
    e1.petName = "猫猫";
    entries.append(e1);

    ChatLogEntry e2;
    e2.role = "assistant";
    e2.content = "今天天气很好";
    e2.provider = "deepseek";
    e2.model = "deepseek-chat";
    e2.petName = "猫猫";
    entries.append(e2);

    ChatLogEntry e3;
    e3.role = "user";
    e3.content = "讲个笑话";
    e3.provider = "openai";
    e3.model = "gpt-4";
    e3.petName = "狗狗";
    entries.append(e3);

    QList<ChatLogEntry> filtered = ChatLogService::filterEntries(entries, "天气");
    QCOMPARE(filtered.size(), 2);

    QList<ChatLogEntry> filteredByProvider = ChatLogService::filterEntries(entries, "openai");
    QCOMPARE(filteredByProvider.size(), 1);
    QCOMPARE(filteredByProvider.first().content, QString("讲个笑话"));

    QList<ChatLogEntry> filteredByPet = ChatLogService::filterEntries(entries, "狗狗");
    QCOMPARE(filteredByPet.size(), 1);
}

void TestChatLogService::formatEntriesForDisplay()
{
    QList<ChatLogEntry> entries;

    ChatLogEntry user;
    user.timestamp = QDateTime::fromString("2026-01-01T12:00:00", Qt::ISODate);
    user.petId = "cat";
    user.petName = "猫猫";
    user.apiConfigName = "main";
    user.model = "deepseek-chat";
    user.role = "user";
    user.content = "你好";
    entries.append(user);

    ChatLogEntry assistant;
    assistant.timestamp = QDateTime::fromString("2026-01-01T12:00:01", Qt::ISODate);
    assistant.petId = "cat";
    assistant.petName = "噜噜";
    assistant.apiConfigName = "main";
    assistant.model = "deepseek-chat";
    assistant.role = "assistant";
    assistant.content = "你好呀！";
    entries.append(assistant);

    ChatLogEntry system;
    system.timestamp = QDateTime::fromString("2026-01-01T12:00:02", Qt::ISODate);
    system.petId = "cat";
    system.petName = "噜噜";
    system.apiConfigName = "main";
    system.model = "deepseek-chat";
    system.role = "system";
    system.content = "联网搜索完成";
    entries.append(system);

    QString display = ChatLogService::formatEntriesForDisplay(entries);
    QVERIFY(display.contains("我"));
    QVERIFY(display.contains("噜噜："));
    QVERIFY(display.contains("系统"));
    QVERIFY(display.contains("你好"));
    QVERIFY(display.contains("你好呀！"));
    QVERIFY(display.contains("联网搜索完成"));
}

#include "moc_test_chatlogservice.cpp"
