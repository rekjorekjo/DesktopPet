#ifndef CHATSETTINGSSERVICE_H
#define CHATSETTINGSSERVICE_H

#include <QString>

class ChatSettingsService
{
public:
    static ChatSettingsService &instance();

    bool load(QString *error = nullptr);
    bool save(QString *error = nullptr) const;

    QString systemPrompt() const;
    void setSystemPrompt(const QString &prompt);

    void resetToDefaultSystemPrompt();

    static QString defaultSystemPrompt();

private:
    ChatSettingsService();

private:
    QString m_systemPrompt;
};

#endif // CHATSETTINGSSERVICE_H
