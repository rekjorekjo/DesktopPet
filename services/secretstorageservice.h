#ifndef SECRETSTORAGESERVICE_H
#define SECRETSTORAGESERVICE_H

#include <QString>

class SecretStorageService
{
public:
    static QString backendName();
    static bool usesSecureStorage();
    static QString securityNotice();

    static QString maskSecret(const QString &secret);
    static bool looksLikeSecret(const QString &text);
    static QString redactSecrets(const QString &text);

private:
    SecretStorageService() = delete;
};

#endif // SECRETSTORAGESERVICE_H
