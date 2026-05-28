#ifndef TEST_SECRETSTORAGESERVICE_H
#define TEST_SECRETSTORAGESERVICE_H

#include <QObject>

class TestSecretStorageService : public QObject
{
    Q_OBJECT

private slots:
    void backendName();
    void usesSecureStorage();
    void maskSecret_data();
    void maskSecret();
    void looksLikeSecret_data();
    void looksLikeSecret();
    void redactSecrets_data();
    void redactSecrets();
};

#endif
