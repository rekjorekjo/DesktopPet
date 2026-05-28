#ifndef TEST_APIPROFILESERVICE_H
#define TEST_APIPROFILESERVICE_H

#include <QObject>
#include <QTemporaryDir>

class TestApiProfileService : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;

    void writeProfilesJson(const QString &content);

private slots:
    void init();
    void cleanup();

    void loadDoesNotUseLegacyQSettingsFallback();
    void setCurrentProfilePersistsOnlyToJson();
    void invalidCurrentProfileStillFallsBackToFirstProfile();
};

#endif // TEST_APIPROFILESERVICE_H
