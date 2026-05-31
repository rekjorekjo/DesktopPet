#ifndef TEST_APICONFIG_H
#define TEST_APICONFIG_H

#include <QObject>

class TestApiConfig : public QObject
{
    Q_OBJECT

private slots:
    void parseTemplateIgnoresCommentLines();
    void customPresetUsesOpenAICompatible();
    void claudePresetHidden();
    void maxTokensUndefined();
    void maxTokens2048();
    void maxTokensZero();
    void maxTokensNegative();
    void maxTokensTooLarge();
    void maxTokensNonNumeric();
};

#endif // TEST_APICONFIG_H
